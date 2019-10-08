#include <stdio.h>
#include <math.h>
#include <iostream>
#include <map>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "fq_common.h"
#include "fq_occlusion.h"

using namespace std;
using namespace cv;

OcclusionDetector::OcclusionDetector(std::string &model_dir)
{
	std::string weights = model_dir + "cofw_model.pb";
	std::string pbtxt = model_dir + "cofw_model.pbtxt";

	m_net = cv::dnn::readNetFromTensorflow(weights, pbtxt);

	int type_num = sizeof(m_occ_str) / sizeof(m_occ_str[0]);
	for (int i = 0; i < type_num; i++)
	{
		m_occlusion.insert(std::pair<std::string, float>(m_occ_str[i], 0.5));
	}
}

int OcclusionDetector::detect(cv::Mat &img, cv::Rect &occlusion_box, int input_size, float *prob)
{
	Mat gray;

	if (img.channels() > 1)
    {
		cvtColor(img, gray, COLOR_BGR2GRAY);
    }
    else
    {
        gray = img;
    }


	int margin = 8;

	cv::Rect box_lt = cv::Rect(occlusion_box.x, occlusion_box.y, occlusion_box.width - margin, occlusion_box.height - margin);
	cv::Rect box_rt = cv::Rect(occlusion_box.x + margin, occlusion_box.y, occlusion_box.width - margin, occlusion_box.height - margin);
	cv::Rect box_lb = cv::Rect(occlusion_box.x, occlusion_box.y + margin, occlusion_box.width - margin, occlusion_box.height - margin);
	cv::Rect box_rb = cv::Rect(occlusion_box.x + margin, occlusion_box.y + margin, occlusion_box.width - margin, occlusion_box.height - margin);
	cv::Rect box_ct = cv::Rect(occlusion_box.x + margin / 2, occlusion_box.y + margin/2, occlusion_box.width - margin/2, occlusion_box.height - margin/2);

	std::vector<cv::Rect> boxes;
	boxes.clear();
	boxes.push_back(box_lt);
	boxes.push_back(box_rt);
	boxes.push_back(box_lb);
	boxes.push_back(box_rb);
	boxes.push_back(box_ct);

	int class_num = sizeof(m_occ_str) / sizeof(m_occ_str[0]);

	Mat pred_sum = Mat::zeros(1, class_num, CV_32F);

	for (int i = 0; i < boxes.size(); i++)
	{
		Mat crop_img = gray(boxes[i]);
		Mat input_img;
		cv::resize(crop_img, input_img, cv::Size(input_size, input_size));
		//cv::imshow("input_img", input_img);
		//cv::waitKey(0);

		Mat inputBlob = cv::dnn::blobFromImage(input_img, 0.00390625f, Size(input_size, input_size), Scalar(), false,false);
		m_net.setInput(inputBlob, "conv2d_1_input");
		Mat pred = m_net.forward("dense_3/Sigmoid");

		//cout << pred << endl;

		pred_sum = pred_sum + pred;
	}

	//cout << pred_sum << endl;

	for (int i = 0; i < class_num; i++)
	{
		m_occlusion[m_occ_str[i]] = pred_sum.at<float>(i) / boxes.size();
		//cout << m_occlusion[m_occ_str[i]] << endl;
	}

	*prob = pred_sum.at<float>(0) / boxes.size();

	return 0;
}

void OcclusionDetector::draw(cv::Mat &img, cv::Point &org, cv::Scalar &color)
{
	int thickness = 2;
	char text[64] = {0};

	for (int i = 0; i < sizeof(m_occ_str) / sizeof(m_occ_str[0]); i++)
	{
		snprintf(text, sizeof(text), "%s: %.3f", m_occ_str[i].c_str(), m_occlusion[m_occ_str[i]]);
		cv::putText(img, text, cv::Point(org.x, org.y + 30 * i), cv::FONT_HERSHEY_COMPLEX,
					0.8, color, thickness, LINE_8, false);
	}
}

float OcclusionDetector::at(const std::string &name)
{
    return m_occlusion.at(name);
}

void OcclusionDetector::add(const std::string &name, float score)
{
    m_occlusion.insert(std::pair<std::string, float>(name, score));
}

