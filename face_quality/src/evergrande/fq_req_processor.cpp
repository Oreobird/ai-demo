#include "fq_req_processor.h"
#include "fq_req_mgt.h"
#include "base_net.h"
#include "base_string.h"
#include "base_logger.h"
#include "base_uid.h"
#include "base_utility.h"

#include "fq_msg.h"
#include "fq_msg_oper.h"
#include "fq_conf_mgt.h"
#include "fq_detector_mgt.h"

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

	Request_Ptr req;
	nRet = g_req_mgt->get_req(req);
	if(nRet != 0)
	{
		return 0;
	}

	MatCodecs mat_codecs;
	fa_dbg("req msg len:%ld\n", strlen(req->_req.c_str()));

	Mat img = mat_codecs.base64_to_mat(req->_req);
	fa_dbg("img width:%d ,height:%d\n", img.cols, img.rows);

	XCP_LOGGER_INFO(&g_logger, "img width:%d ,height:%d\n", img.cols, img.rows);

	EvgJson evg_json;
	std::string json_str;

	if (img.empty())
	{
		XCP_LOGGER_ERROR(&g_logger, "img is empty.\n");
		std::string img_data_str = "";
		evg_json.gen_exception_json(0, img_data_str, json_str);
	}
	else
	{
		//imshow("img", img);
		//waitKey(0);

		std::vector<landmark_t> mark;

		if (true)
		{
			//base::Thread_Mutex_Guard guard(m_mutex);
			detector_t *detector = NULL;

			nRet = g_detector_mgt->get_detector(&detector);
			if (nRet != 0)
			{
				XCP_LOGGER_INFO(&g_logger, "get detector failed\n");
				return 0;
			}

			int face_num = 0;
			if (detector && detector->face_detector)
			{
				detector->face_detector->face_detect(img);

				detector->face_detector->landmark_detect(img, &mark);

				if (face_num == 1)
				{
					float occ_prob;
					detector->occ_detector->detect(img, mark[0].emotion_box, 96, &occ_prob);
					detector->quality->set("occlusion", occ_prob);

					float emt_prob;
					detector->emt_recognizer->recognize(img, mark[0].emotion_box, 48, &emt_prob);
					detector->quality->set("emotion", emt_prob);

					detector->quality->cal_all_scores(img, &mark[0]);

				}

				detector->face_detector->landmark_draw(img, cv::Scalar(0, 0, 255), false, result_img_path);

				std::string img_data_str = mat_codecs.mat_to_base64(img);

				if (face_num == 1)
				{
					evg_json.gen_face_json(&mark[0], detector->quality, img_data_str, json_str);
				}
				else
				{
					evg_json.gen_exception_json(face_num, img_data_str, json_str);
				}
			}

			g_detector_mgt->put_detector(&detector);
		}
	}
	//cout << json_str << endl;

	#if 1
	int buf_len = json_str.length() + 1;

	//fa_dbg("json_str len:%d\n", buf_len);

	int msg_len = sizeof(evg_msg_t) + buf_len;

	evg_msg_t *msg = (evg_msg_t *) new char[msg_len];
	if (msg == NULL)
	{
		XCP_LOGGER_INFO(&g_logger, "Malloc fail!\n");
		return -1;
	}

	memset(msg, 0, msg_len);
	msg->buf_len = htonl(buf_len);
	memcpy(msg->buf, json_str.c_str(), buf_len);

	Msg_Oper::send_msg(req->_fd, (char *)msg, msg_len);
	if (msg)
	{
		delete [] msg;
		msg = NULL;
	}
	#else
	Msg_Oper::send_msg(req->_fd, json_str);
	#endif
	return 0;

}
