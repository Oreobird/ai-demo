#include "req_processor.h"
#include "protocol.h"
#include "req_mgt.h"
#include "base_net.h"
#include "base_string.h"
#include "base_logger.h"
#include "base_uid.h"
#include "base_utility.h"

#include "msg_oper.h"
#include "conf_mgt.h"
#include "detector_mgt.h"

#include <arpa/inet.h>

extern Logger g_logger;
extern StSysInfo g_sysInfo;
extern Req_Mgt *g_req_mgt;
extern Detector_Mgt *g_detector_mgt;

using namespace cv;
using namespace std;

#define MAX_CLIENT 100

extern Logger g_logger;

std::string result_img_path;


Req_Processor::Req_Processor()
{

}


Req_Processor::~Req_Processor()
{

}

int Req_Processor::do_init(void *args)
{
	return 0;
}

int Req_Processor::svc()
{
	int nRet = 0;
	std::string err_info = "";
	std::string json_str;

	Request_Ptr req;
	nRet = g_req_mgt->get_req(req);
	if(nRet != 0)
	{
		return 0;
	}

	std::string video_path = "";
	std::string img_data = "";
	std::vector<cv::Point2d> pts_ratio;
	pts_ratio.clear();

	nRet = XProtocol::req_parse(req->_req, img_data, video_path, pts_ratio, err_info);
	if (nRet != 0)
	{
		XCP_LOGGER_INFO(&g_logger, "it is invalid req, ret:%d, err_info:%s, req:%s\n", nRet, err_info.c_str(), req->to_string().c_str());
		json_str = XProtocol::gen_failed_result(ERR_SYSTEM, err_info);
	}
	else
	{
		detector_t *detector = NULL;

		nRet = g_detector_mgt->get_detector(&detector);
		if (nRet != 0 || detector == NULL || detector->obj_detector == NULL)
		{
			XCP_LOGGER_INFO(&g_logger, "get detector failed\n");
			json_str = XProtocol::gen_failed_result(ERR_SYSTEM, err_info);
		}
		else if (!img_data.empty())
		{
			MatCodecs mat_codecs;
			Mat img = mat_codecs.base64_to_mat(img_data);

			dbg("req len:%ld, img width:%d ,height:%d\n", strlen(req->_req.c_str()), img.cols, img.rows);

			XCP_LOGGER_INFO(&g_logger, "img width:%d ,height:%d\n", img.cols, img.rows);

			if (img.empty())
			{
				XCP_LOGGER_ERROR(&g_logger, "img is empty.\n");
				json_str = XProtocol::gen_failed_result(ERR_SYSTEM, "img decode failed.");
			}
			else
			{
				bool alarm = false;
				int ret = detector->obj_detector->detect(img, pts_ratio, 1, false, alarm);
				if (ret < 0)
				{
					XCP_LOGGER_INFO(&g_logger, "object detect failed\n");
					json_str = XProtocol::gen_failed_result(ERR_SYSTEM, "object detect error.");
				}
				else
				{
					std::string img_data_str = mat_codecs.mat_to_base64(img);
					json_str = XProtocol::gen_detect_result(img_data_str, alarm, ERR_SUCCESS);
				}
			}
		}
		else if (!video_path.empty())
		{
			bool err = false;
			cv::Mat frame;

			cv::VideoCapture cap(video_path);

			int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
			int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
			long frame_total = static_cast<long>(cap.get(cv::CAP_PROP_FRAME_COUNT));

			int codec = cv::VideoWriter::fourcc('H', '2', '6', '4');
			int fps = cap.get(cv::CAP_PROP_FPS);

			//std::cout << "fps:" << fps << std::endl;
			time_t currtime = time(NULL);
			tm* p = localtime(&currtime);
			char filename[128] = {0};
			char filepath[256] = {0};

	    	sprintf(filename, "/video/output_%d%02d%02d%02d%02d%02d.mp4",p->tm_year+1900,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
			sprintf(filepath, "/data/upload%s", filename);

			std::string output_path(filepath);
			VideoWriter video(output_path, codec, fps, cv::Size(frame_width, frame_height));

			long frame_count = 0;
			int progress = 0;
			int pre_progress = 0;

			while (cap.read(frame))
			{
				if (frame.empty())
				{
					std::cout << "frame empty" << std::endl;
					err = true;
					break;
				}

				frame_count++;
				#if 0
				if (frame_count % 5 != 0)
				{
					continue;
				}
				#endif
				bool alarm = false;
				int ret = detector->obj_detector->detect(frame, pts_ratio, 1, false, alarm);
				if (ret < 0)
				{
					XCP_LOGGER_INFO(&g_logger, "object detect failed\n");
					err = true;
					break;
				}

				video.write(frame);

				progress = frame_count * 100 / frame_total;

				if (progress - pre_progress >= 2 || progress == 100)
				{
					std::cout << "total frame: " << frame_total << ", progress:" << progress << "%" << ", pre_progress:"<<pre_progress<<"%"<< std::endl;
					pre_progress = progress;
					#if 1
					json_str = XProtocol::gen_video_detect_progress(progress, ERR_SUCCESS);
					nRet = Msg_Oper::send_evg_msg(req->_fd, json_str);
					if (nRet < 0)
					{
						err = true;
						break;
					}
					#endif
				}

			}

			if (err)
			{
				json_str = XProtocol::gen_failed_result(ERR_SYSTEM, "object detect error.");
			}
			else
			{
				g_detector_mgt->put_detector(&detector);
				json_str = XProtocol::gen_video_detect_result(filename, ERR_SUCCESS);
			}
		}
	}

	#if 1
	Msg_Oper::send_evg_msg(req->_fd, json_str);
	#else
	Msg_Oper::send_msg(req->_fd, json_str);
	#endif
	return 0;

}
