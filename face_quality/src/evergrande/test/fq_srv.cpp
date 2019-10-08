#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/dnn.hpp>

#include "fq_common.h"
#include "fq_quality.h"
#include "fq_timing.h"
#include "fq_emotion.h"
#include "fq_occlusion.h"
#include "fq_detector.h"
#include "fq_json.h"
#include "fq_msg.h"
#include "fq_codecs.h"


using namespace std;
//using namespace base;
using namespace cv;

#define MAX_CLIENT 100

std::string model_dir = "/model/";
std::string json_path;
std::string img_path;
std::string result_img_path;


typedef struct _detector
{
	Detector *face_detector;
	OcclusionDetector *occ_detector;
	EmotionRecognizer *emt_recognizer;
	Quality *quality;
} detector_t;

static detector_t detectors;

static int set_socket_nonblock(int fd)
{
    int flags = -1;
    int ret = -1;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        fa_dbg("fcntl get socket error:%s\n", strerror(errno));
        return -1;
    }

    flags |= O_NONBLOCK;

    ret = fcntl(fd, F_SETFL, flags);
    if (ret < 0)
    {
        fa_dbg("fcntl set socket error:%s\n", strerror(errno));
        return -1;
    }

    return 0;
}


static void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

static void modify_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

static int epoll_handle_connect(int epollfd, int fd)
{
    struct sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);
    int clifd = -1;

    while ((clifd = accept(fd, (struct sockaddr *)&addr, &addr_len)) > 0)
    {
        fa_dbg("accept connection from: %s\n", inet_ntoa(*((struct in_addr*)&addr.sin_addr.s_addr)));

        set_socket_nonblock(clifd);
        add_event(epollfd, clifd, EPOLLIN | EPOLLET);
    }
    return 0;
}

int msg_process(evg_msg_t *in_msg, evg_msg_t **out_msg)
{
	// in_buf to Mat
	if (in_msg == NULL || in_msg->buf_len <= 0)
	{
		fa_dbg("msg_process in_msg is NULL\n");
		return -1;
	}

	MatCodecs mat_codecs;
	fa_dbg("in_msg buf strlen:%ld\n", strlen(in_msg->buf));
	Mat img = mat_codecs.base64_to_mat(in_msg->buf);
	#if 0
	vector<char> img_data(in_msg->buf, in_msg->buf + in_msg->buf_len);
	Mat img = cv::imdecode(img_data, CV_LOAD_IMAGE_COLOR);
	#endif

	fa_dbg("img width:%d ,height:%d\n", img.cols, img.rows);

	if (img.empty())
	{
		return -1;
	}
	//imshow("img", img);
	//waitKey(0);

	EvgJson evg_json;

	std::vector<landmark_t> mark;

	int face_num = detectors.face_detector->face_detect(img);

	detectors.face_detector->landmark_detect(img, &mark);

	if (face_num == 1)
	{
		detectors.quality->cal_all_scores(img, &mark[0]);

		float occ_prob;
		detectors.occ_detector->detect(img, &(mark[0].emotion_box), 96, &occ_prob);
		detectors.quality->set("occlusion", occ_prob);

		float emt_prob;
		detectors.emt_recognizer->recognize(img, &(mark[0].emotion_box), 48, &emt_prob);
		detectors.quality->set("emotion", emt_prob);
	}

	detectors.face_detector->landmark_draw(img, cv::Scalar(0, 0, 255), false, result_img_path);

	#if 0
    std::vector<uchar> data_encode;
    cv::imencode(".jpg", img, data_encode);

    std::string img_data_str(data_encode.begin(), data_encode.end());
    #endif

    std::string img_data_str = mat_codecs.mat_to_base64(img);

    fa_dbg("===img_data_str len:%ld\n", img_data_str.length());
	std::string json_str;

	if (face_num == 1)
	{
		evg_json.gen_face_json(&mark[0], detectors.quality, img_data_str, json_str);
	}
	else
	{
		evg_json.gen_exception_json(face_num, img_data_str, json_str);
	}

	//cout << json_str << endl;

	int buf_len = json_str.length() + 1;
	int msg_len = (buf_len + sizeof(evg_msg_t)) * sizeof(char);
	*out_msg = (evg_msg_t *)malloc(msg_len);
	if (*out_msg == NULL)
	{
		fa_dbg("out_msg payload malloc fail\n");
		return -1;
	}

	memset(*out_msg, 0, msg_len);
	(*out_msg)->buf_len = buf_len;
	memcpy((*out_msg)->buf, json_str.c_str(), buf_len);

	fa_dbg("out_msg buf len:%d\n", buf_len);

	return 0;
}

static int epoll_read(int epollfd, int clifd, evg_msg_t **msg)
{
    if (clifd >= 0)
    {
        int nread = 0;
		int buf_len = 0;
        nread = read(clifd, (char *)&buf_len, sizeof(buf_len));
        buf_len = ntohl(buf_len);
		fa_dbg("buf_len:%d\n", buf_len);

		int msg_len = (buf_len + sizeof(evg_msg_t) + 1) * sizeof(char);
		*msg = (evg_msg_t *)malloc(msg_len);
		if (*msg == NULL)
		{
			fa_dbg("msg malloc fail\n");
			return -1;
		}

		memset(*msg, 0, msg_len);
		(*msg)->buf_len = buf_len;

        int data_left = (*msg)->buf_len;
		char *ptr = (*msg)->buf;
		nread = 0;

        while (data_left > 0)
        {
        	nread = read(clifd, ptr, data_left);
        	if (nread < 0)
        	{
        		if (errno == EINTR || errno == EAGAIN)
        		{
        			nread = 0;
        		}
        		else
        		{
        			fa_dbg("errno:%s\n", strerror(errno));
        			break;
        		}
        	}
        	else if (nread == 0)
        	{
        		break;
        	}

            data_left -= nread;
            ptr += nread;
        }

		fa_dbg("need read:%d, read data len:%d\n", (*msg)->buf_len, (*msg)->buf_len - data_left);

        if (nread < 0 && errno != EAGAIN)
        {
            fa_dbg("read data failed\n");
            close(clifd);
            delete_event(epollfd, clifd, EPOLLIN | EPOLLET);
            return -1;
        }
        else if (nread == 0)
        {
            fa_dbg("client disconnted\n");
            close(clifd);
            delete_event(epollfd, clifd, EPOLLIN | EPOLLET);
            return -1;
        }

        fa_dbg("recv data\n");

    }

    return 0;
}

static int epoll_write(int epollfd, int clifd, evg_msg_t *msg)
{
    if (clifd >= 0 && msg != NULL)
    {
        int msg_len = (sizeof(evg_msg_t) + msg->buf_len) * sizeof(char);
        int data_left = msg_len;
        int nwrite = 0;
        char *ptr = (char *)msg;
		msg->buf_len = htonl(msg->buf_len);
        while (msg_len > 0)
        {
            nwrite = write(clifd, ptr, data_left);
         	if (nwrite <= 0)
         	{
         		if (nwrite < 0 && (errno == EINTR || errno == EAGAIN))
         		{
         			nwrite = 0;
         		}
         		else
         		{
         			fa_dbg("write errno:%s\n", strerror(errno));
         			break;
         		}
         	}

            data_left -= nwrite;
            ptr += nwrite;
        }

        fa_dbg("need write len:%d, write data len:%d\n", msg_len, msg_len - data_left);

        if (nwrite < 0)
        {
            fa_dbg("write data failed\n");
            close(clifd);
            delete_event(epollfd, clifd, EPOLLOUT | EPOLLET);
            return -1;
        }
        else if (nwrite == 0)
        {
            fa_dbg("client disconnted\n");
            close(clifd);
            delete_event(epollfd, clifd, EPOLLOUT | EPOLLET);
            return -1;
        }

        fa_dbg("send data\n");

        modify_event(epollfd, clifd, EPOLLIN | EPOLLET);
    }
    return 0;
}

int main(int argc, char** argv)
{
	char prog_path[256] = {0};
	char *p = strrchr(argv[0], '/');
	strncpy(prog_path, argv[0], p - argv[0]);
	model_dir = prog_path + model_dir;

	// detectors init
	DlibLandmarkDetector *dlib_lm_detector = new DlibLandmarkDetector(model_dir);
	Detector face_detector(dlib_lm_detector, model_dir);
	OcclusionDetector occ_detector(model_dir);
	EmotionRecognizer emt_recognizer(model_dir);
	Quality quality(model_dir);

	detectors.face_detector = &face_detector;
	detectors.occ_detector = &occ_detector;
	detectors.emt_recognizer = &emt_recognizer;
	detectors.quality = &quality;

	// server network process
	signal(SIGPIPE, SIG_IGN);

    int sockfd = -1;
    int optval = 1;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockfd < 0)
    {
        fa_dbg("create socket failed\n");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        fa_dbg("setsockopt failed\n");
        close(sockfd);
        return -1;
    }

    set_socket_nonblock(sockfd);

    struct sockaddr_in addr;
    int ret = -1;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(6666);

    ret = bind(sockfd, (struct sockaddr*)&(addr), sizeof(addr));
    if (ret < 0)
    {
        fa_dbg("bind addr failed\n");
        return -1;
    }

    ret = listen(sockfd, MAX_CLIENT);
    if (ret < 0)
    {
        fa_dbg("listen to port failed\n");
        return -1;
    }

	struct epoll_event events[MAX_CLIENT];
    int i, nfds, epollfd = -1;

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        fa_dbg("epoll_create1 failed\n");
        return -1;
    }

    add_event(epollfd, sockfd, EPOLLIN | EPOLLET);

	evg_msg_t *in_msg = NULL;
	evg_msg_t *out_msg = NULL;

	while (true)
	{
		fa_dbg("epoll waiting ...\n");
		nfds = epoll_wait(epollfd, events, MAX_CLIENT, -1);
		if (nfds == -1)
		{
			fa_dbg("epoll_wait err\n");
			return -1;
		}

		for (i = 0; i < nfds; i++)
		{
			if (events[i].data.fd == sockfd && (events[i].events & EPOLLIN))
			{
				ret = epoll_handle_connect(epollfd, sockfd);
				if (ret < 0)
				{
					fa_dbg("epoll_handle_connect error\n");
				}
			}
			else
			{
			    if (events[i].events & EPOLLIN)
			    {
			        ret = epoll_read(epollfd, events[i].data.fd, &in_msg);
					if (ret == 0)
					{
						if (in_msg)
						{
							int result = msg_process(in_msg, &out_msg);

							free(in_msg);
							in_msg = NULL;

							modify_event(epollfd, events[i].data.fd, EPOLLOUT | EPOLLET);
						}
					}
			    }
			    else if (events[i].events & EPOLLOUT)
			    {
			        if (out_msg)
					{
						ret = epoll_write(epollfd, events[i].data.fd, out_msg);
						free(out_msg);
						out_msg = NULL;
					}
			    }
			}
		}
	}

	return 0;
}
