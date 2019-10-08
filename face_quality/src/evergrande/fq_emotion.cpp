#include <stdio.h>
#include <math.h>
#include <iostream>
#include <map>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "fq_common.h"
#include "fq_emotion.h"

using namespace std;
using namespace cv;

EmotionRecognizer::EmotionRecognizer(std::string &model_dir)
{
	std::string weights = model_dir + "fer_model.pb";
	std::string pbtxt = model_dir + "fer_model.pbtxt";

	m_net = cv::dnn::readNetFromTensorflow(weights, pbtxt);

	int type_num = sizeof(m_emo_str) / sizeof(m_emo_str[0]);
	for (int i = 0; i < type_num; i++)
	{
		m_emotion.insert(std::pair<std::string, float>(m_emo_str[i], 0.5));
	}
}

int EmotionRecognizer::recognize(cv::Mat &img, cv::Rect &emotion_box, int input_size, float *prob)
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

	int margin = 12;

	cv::Rect box_lt = cv::Rect(emotion_box.x, emotion_box.y, emotion_box.width - margin, emotion_box.height - margin);
	cv::Rect box_rt = cv::Rect(emotion_box.x + margin, emotion_box.y, emotion_box.width - margin, emotion_box.height - margin);
	cv::Rect box_lb = cv::Rect(emotion_box.x, emotion_box.y + margin, emotion_box.width - margin, emotion_box.height - margin);
	cv::Rect box_rb = cv::Rect(emotion_box.x + margin, emotion_box.y + margin, emotion_box.width - margin, emotion_box.height - margin);
	cv::Rect box_ct = cv::Rect(emotion_box.x + margin / 2, emotion_box.y + margin/2, emotion_box.width - margin/2, emotion_box.height - margin/2);

	std::vector<cv::Rect> boxes;
	boxes.clear();
	boxes.push_back(box_lt);
	boxes.push_back(box_rt);
	boxes.push_back(box_lb);
	boxes.push_back(box_rb);
	boxes.push_back(box_ct);

	int class_num = sizeof(m_emo_str) / sizeof(m_emo_str[0]);

	Mat pred_sum = Mat::zeros(1, class_num, CV_32F);

	for (int i = 0; i < boxes.size(); i++)
	{
		Mat crop_img = gray(boxes[i]);
		Mat input_img;
		cv::resize(crop_img, input_img, cv::Size(input_size, input_size));

		//cv::imshow("input_img", input_img);
		//cv::waitKey(0);

		Mat inputBlob = cv::dnn::blobFromImage(input_img, 0.00390625f, Size(input_size, input_size), Scalar(), false,false);
		m_net.setInput(inputBlob, "conv2d_input");
		Mat pred = m_net.forward("dense_2/Softmax");

		//cout << pred << endl;

		pred_sum = pred_sum + pred;
	}

	//cout << pred_sum << endl;

	for (int i = 0; i < class_num; i++)
	{
		m_emotion[m_emo_str[i]] = pred_sum.at<float>(i) / boxes.size();
		//cout << m_emotion[m_emo_str[i]] << endl;
	}

	*prob = pred_sum.at<float>(1) / boxes.size();

	return 0;
}

void EmotionRecognizer::draw(cv::Mat &img, cv::Point &org, cv::Scalar &color)
{
	int thickness = 2;
	char text[64] = {0};

	for (int i = 0; i < sizeof(m_emo_str) / sizeof(m_emo_str[0]); i++)
	{
		snprintf(text, sizeof(text), "%s: %.3f", m_emo_str[i].c_str(), m_emotion[m_emo_str[i]]);
		cv::putText(img, text, cv::Point(org.x, org.y + 30 * i), cv::FONT_HERSHEY_COMPLEX,
					0.8, color, thickness, LINE_8, false);
	}
}


float EmotionRecognizer::at(const std::string &name)
{
    return m_emotion.at(name);
}