#ifndef __OBJECT_DETECT_H__
#define __OBJECT_DETECT_H__

#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

class ObjDetector
{
public:
	ObjDetector(const std::string &model_file, const std::string &cfg_file, cv::Size img_size=cv::Size(320, 320), float conf=0.5, float nms=0.4);

	~ObjDetector();

    void draw_alarm_line(cv::Mat &frame, const std::vector<cv::Point> &points, cv::Scalar color);
    bool alarm_check(const cv::Rect &box, const std::vector<cv::Point> &points, int direction);

    int detect(cv::Mat &frame, const std::vector<cv::Point2d> &points_ratio,
                                int direction, bool show, bool &alarm);

	int detect(cv::Mat &frame, std::vector<cv::Rect2d> &bboxes);

private:
	int parse_class_boxes(const cv::Mat& frame, const std::vector<cv::Mat>& outs,
								std::vector<int>& classIds,
								std::vector<float>& confidences,
								std::vector<cv::Rect>& boxes);
	void drawPred(cv::Mat& frame, int classId, float conf,
								int left, int top, int right, int bottom, cv::Scalar &color);
    void draw_alarm_text(cv::Mat &frame, bool alarm);

private:
	float m_conf_threshold;
	float m_nms_threshold;
	cv::Size m_img_size;
	cv::dnn::Net m_net;
};


#endif
