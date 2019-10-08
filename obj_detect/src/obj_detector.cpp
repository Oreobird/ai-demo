#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "obj_detector.h"
#include "detector_mgt.h"

using namespace cv;
using namespace std;

static const char *coco_classes_str[] = {"person","bicycle","car","motorcycle","airplane","bus","train","truck","boat","traffic light","fire hydrant","stop sign","parking meter","bench","bird","cat","dog","horse","sheep","cow","elephant","bear","zebra","giraffe","backpack","umbrella","handbag","tie","suitcase","frisbee","skis","snowboard","sports ball","kite","baseball bat","baseball glove","skateboard","surfboard","tennis racket","bottle","wine glass","cup","fork","knife","spoon","bowl","banana","apple","sandwich","orange","broccoli","carrot","hot dog","pizza","donut","cake","chair","couch","potted plant","bed","dining table","toilet","tv","laptop","mouse","remote","keyboard","cell phone","microwave","oven","toaster","sink","refrigerator","book","clock","vase","scissors","teddy bear","hair drier","toothbrush"};
static const int target_classes[] = {0, 16}; //coco_ids: {"person", "dog"}

static bool is_target_class(int class_id);

ObjDetector::ObjDetector(const std::string &model_file, const std::string &cfg_file, cv::Size img_size, float conf, float nms):m_img_size(img_size), m_conf_threshold(conf), m_nms_threshold(nms)
{
    m_net = cv::dnn::readNet(model_file, cfg_file);
    m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

ObjDetector::~ObjDetector()
{
}

int ObjDetector::parse_class_boxes(const cv::Mat& frame, const std::vector<cv::Mat>& outs,
										std::vector<int>& classIds,
										std::vector<float>& confidences,
										std::vector<cv::Rect>& boxes)
{
    static std::vector<int> outLayers = m_net.getUnconnectedOutLayers();
    static std::string outLayerType = m_net.getLayer(outLayers[0])->type;

    if (m_net.getLayer(0)->outputNameToIndex("im_info") != -1)  // Faster-RCNN or R-FCN
    {
        // Network produces output blob with a shape 1x1xNx7 where N is a number of
        // detections and an every detection is a vector of values
        // [batchId, classId, confidence, left, top, right, bottom]
        CV_Assert(outs.size() == 1);
        float* data = (float*)outs[0].data;
        for (size_t i = 0; i < outs[0].total(); i += 7)
        {
            float confidence = data[i + 2];
            if (confidence > m_conf_threshold)
            {
                int left = (int)data[i + 3];
                int top = (int)data[i + 4];
                int right = (int)data[i + 5];
                int bottom = (int)data[i + 6];
                int width = right - left + 1;
                int height = bottom - top + 1;
                classIds.push_back((int)(data[i + 1]) - 1);  // Skip 0th background class id.
                boxes.push_back(cv::Rect(left, top, width, height));
                confidences.push_back(confidence);
            }
        }
    }
    else if (outLayerType == "DetectionOutput")
    {
        // Network produces output blob with a shape 1x1xNx7 where N is a number of
        // detections and an every detection is a vector of values
        // [batchId, classId, confidence, left, top, right, bottom]

        CV_Assert(outs.size() == 1);
        float* data = (float*)outs[0].data;
        for (size_t i = 0; i < outs[0].total(); i += 7)
        {
            float confidence = data[i + 2];
            if (confidence > m_conf_threshold)
            {
                int left = (int)(data[i + 3] * frame.cols);
                int top = (int)(data[i + 4] * frame.rows);
                int right = (int)(data[i + 5] * frame.cols);
                int bottom = (int)(data[i + 6] * frame.rows);
                int width = right - left + 1;
                int height = bottom - top + 1;
                classIds.push_back((int)(data[i + 1]) - 1);  // Skip 0th background class id.
                boxes.push_back(cv::Rect(left, top, width, height));
                confidences.push_back(confidence);
            }
        }
    }
    else if (outLayerType == "Region")
    {
        for (size_t i = 0; i < outs.size(); ++i)
        {
            // Network produces output blob with a shape NxC where N is a number of
            // detected objects and C is a number of classes + 4 where the first 4
            // numbers are [center_x, center_y, width, height]

            float* data = (float*)outs[i].data;
            for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
            {
                cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
                cv::Point classIdPoint;
                double confidence;
                minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
                if (confidence > m_conf_threshold
                	&& is_target_class(classIdPoint.x))
                {
                    int centerX = (int)(data[0] * frame.cols);
                    int centerY = (int)(data[1] * frame.rows);
                    int width = (int)(data[2] * frame.cols);
                    int height = (int)(data[3] * frame.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }
    }
    else
    {
        CV_Error(Error::StsNotImplemented, "Unknown output layer type: " + outLayerType);
    }

    return 0;
}

void ObjDetector::drawPred(cv::Mat& frame, int classId, float conf,
								int left, int top, int right, int bottom,
								cv::Scalar &color)
{
	//std::cout << "left:"<<left<<", right:"<<right<<", top:"<<top<<", bottom:"<<bottom<<std::endl;
    cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), color, 3);

	#if 0
    std::string label = format("%.2f", conf);

    CV_Assert(classId < (int)sizeof(coco_classes_str)/sizeof(coco_classes_str[0]));
    char text[256] = {0};
    snprintf(text, sizeof(text), "%s: %s", coco_classes_str[classId], label.c_str());

    int baseLine;
    cv::Size labelSize = cv::getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 2, &baseLine);

    top = max(top, labelSize.height);
    cv::rectangle(frame, cv::Point(left, top - labelSize.height),
              cv::Point(left + labelSize.width, top + baseLine), cv::Scalar::all(255), FILLED);
    cv::putText(frame, text, cv::Point(left, top), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar());
    #endif
}


static bool is_target_class(int class_id)
{
	bool ret = false;
	int class_num = sizeof(target_classes) / sizeof(target_classes[0]);

	for (int i = 0; i < class_num; i++)
	{
		if (class_id == target_classes[i])
		{
			ret = true;
			break;
		}
	}

	return ret;
}

static bool line_intersection(const std::vector<cv::Point>& line1, const std::vector<cv::Point>& line2)
{
	CV_Assert(line1.size() == line2.size());
	CV_Assert(line1.size()==2);

	cv::Point point1_11 = line2[0] - line1[0];
	cv::Point point1_12 = line2[1] - line1[0];
	cv::Point point1_21 = line2[0] - line1[1];
	cv::Point point1_22 = line2[1] - line1[1];

	cv::Point point2_11 = line1[0] - line2[0];
	cv::Point point2_12 = line1[1] - line2[0];
	cv::Point point2_21 = line1[0] - line2[1];
	cv::Point point2_22 = line1[1] - line2[1];

	return (point1_11.cross(point1_12) * point1_21.cross(point1_22) < 0
			&& point2_11.cross(point2_12) * point2_21.cross(point2_22) < 0);
}

static bool line_rect_intersection(const std::vector<cv::Point>& line, const cv::Rect& rect)
{
	CV_Assert(line.size()==2);

	//line inside rect
	if (line[0].inside(rect) && line[1].inside(rect))
	{
		return true;
	}

	//line intersect with one rect edge at least
	std::vector<cv::Point> edge;
	edge.clear();
	edge.push_back(cv::Point(rect.x, rect.y));
	edge.push_back(cv::Point(rect.x, rect.y + rect.height - 1));
	if (line_intersection(edge, line))
	{
		return true;
	}

	edge.clear();
	edge.push_back(cv::Point(rect.x + rect.width - 1, rect.y));
	edge.push_back(cv::Point(rect.x + rect.width - 1, rect.y + rect.height - 1));
	if (line_intersection(edge, line))
	{
		return true;
	}

	edge.clear();
	edge.push_back(cv::Point(rect.x, rect.y));
	edge.push_back(cv::Point(rect.x + rect.width - 1, rect.y));
	if (line_intersection(edge, line))
	{
		return true;
	}

	edge.clear();
	edge.push_back(cv::Point(rect.x, rect.y + rect.height - 1));
	edge.push_back(cv::Point(rect.x + rect.width - 1, rect.y+rect.height - 1));
	if (line_intersection(edge, line))
	{
		return true;
	}

	return false;
}


void ObjDetector::draw_alarm_line(cv::Mat &frame, const std::vector<cv::Point> &points, cv::Scalar color)
{
	int thickness=2;
	int lineType=8;

	for (int i = 1; i < points.size(); i++)
	{
		cv::line(frame, points[i - 1], points[i], color, thickness, lineType);
	}
}

void ObjDetector::draw_alarm_text(cv::Mat &frame, bool alarm)
{
	cv::Scalar text_color;
	int font_face = cv::FONT_HERSHEY_COMPLEX;
	double font_scale = 0.8;
	int thickness = 2;
	int baseline;
	std::string text;
	cv::Size text_size;
	cv::Scalar color;
	cv::Point origin;

	if (alarm)
	{
		text = "Alarm!!!";
		text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
		color = cv::Scalar(0, 0, 255);
	}
	else
	{
		text = "Normal";
		text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
		color = cv::Scalar(0, 255, 0);
	}

	origin.x = frame.cols - text_size.width - 8;
	origin.y = text_size.height + 8;

	cv::putText(frame, text, origin, font_face, font_scale, color, thickness, 8);
}

//direction: 0-left,up,inside;
//			 1-right,down,outside
bool ObjDetector::alarm_check(const cv::Rect &box, const std::vector<cv::Point> &points, int direction)
{
	bool ret = false;
	int points_num = points.size();
	int max_x = points[0].x, max_y = points[0].y;
	int min_x = points[0].x, min_y = points[0].y;

	for (int i = 1; i < points_num; i++)
	{
		std::vector<cv::Point> line;
		line.clear();
		line.push_back(cv::Point(points[i - 1].x, points[i - 1].y));
		line.push_back(cv::Point(points[i].x, points[i].y));

		if (line_rect_intersection(line, box))
		{
			ret = true;
			break;
		}

		max_x = (points[i].x > max_x) ? points[i].x : max_x;
		max_y = (points[i].y > max_y) ? points[i].y : max_y;
		min_x = (points[i].x < min_x) ? points[i].x : min_x;
		min_y = (points[i].y < min_y) ? points[i].y : min_y;
	}

	// TODO: obj in area, need to optimize
	#if 0
	if (!ret)
	{
		if (direction == 0)
		{
			if (box.x + box.width - 1 < min_x
				|| box.y + box.height - 1 < min_y)
			{
				ret = true;
			}
		}
		else
		{
			if (box.x > max_x || box.y > max_y)
			{
				ret = true;
			}
		}
	}
	#endif

	return ret;
}

int ObjDetector::detect(cv::Mat &frame, const std::vector<cv::Point2d> &points_ratio,
							int direction, bool show, bool &alarm)
{
	if (frame.empty())
    {
        std::cout << "frame empty" << std::endl;
		return -1;
    }

    //std::cout << m_img_size << std::endl;
	cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 0.00392, m_img_size, cv::Scalar(0,0,0), true, false);

    m_net.setInput(blob);

    if (m_net.getLayer(0)->outputNameToIndex("im_info") != -1)  // Faster-RCNN or R-FCN
    {
        std::cout << "Faster-RCNN" << std::endl;
        cv::resize(frame, frame, m_img_size);
        cv::Mat imInfo = (cv::Mat_<float>(1, 3) << m_img_size.height, m_img_size.width, 1.6f);
        m_net.setInput(imInfo, "im_info");
    }
    std::vector<cv::Mat> outs;
	std::vector<std::string> out_names = m_net.getUnconnectedOutLayersNames();

    m_net.forward(outs, out_names);

	std::vector<int> class_ids;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	parse_class_boxes(frame, outs, class_ids, confidences, boxes);

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, m_conf_threshold, m_nms_threshold, indices);

    //std::cout << "indices.size:"<<indices.size() << std::endl;

	std::vector<cv::Point> points;
	points.clear();
	for (int i = 0; i < points_ratio.size(); i++)
	{
		cv::Point point = cv::Point((int)frame.cols * points_ratio[i].x, (int)frame.rows * points_ratio[i].y);
		points.push_back(point);
	}

	draw_alarm_line(frame, points, cv::Scalar(0, 0, 255)); // red
	bool result = false;

    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        cv::Rect box = boxes[idx];

		result = alarm_check(box, points, direction);

		cv::Scalar color;

		if (result)
		{
			alarm = true;
			color = cv::Scalar(0, 0, 255); // red
		}
		else
		{
			color = cv::Scalar(0, 255, 0); // green
		}

		drawPred(frame, class_ids[idx], confidences[idx], box.x, box.y,
		     	box.x + box.width, box.y + box.height, color);

		if (show)
		{
		    cv::imshow("frame", frame);
		    cv::waitKey(0);
        }
    }

	draw_alarm_text(frame, alarm);

	return 0;
}

int ObjDetector::detect(cv::Mat &frame, std::vector<cv::Rect2d> &bboxes)
{
	if (frame.empty())
    {
        std::cout << "frame empty" << std::endl;
		return -1;
    }

    //std::cout << m_img_size << std::endl;
	cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 0.00392, m_img_size, cv::Scalar(0,0,0), true, false);

    m_net.setInput(blob);

    if (m_net.getLayer(0)->outputNameToIndex("im_info") != -1)  // Faster-RCNN or R-FCN
    {
        std::cout << "Faster-RCNN" << std::endl;
        cv::resize(frame, frame, m_img_size);
        cv::Mat imInfo = (cv::Mat_<float>(1, 3) << m_img_size.height, m_img_size.width, 1.6f);
        m_net.setInput(imInfo, "im_info");
    }

    std::vector<cv::Mat> outs;
	std::vector<std::string> out_names = m_net.getUnconnectedOutLayersNames();

    //std::cout << "start forward"<< std::endl;
    m_net.forward(outs, out_names);
    //std::cout << "forward done"<< std::endl;

	std::vector<int> class_ids;
	std::vector<float> confidences;
	std::vector<cv::Rect> boxes;

	parse_class_boxes(frame, outs, class_ids, confidences, boxes);

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, m_conf_threshold, m_nms_threshold, indices);

    std::cout << "indices.size:"<<indices.size() << std::endl;

	bboxes.clear();
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        cv::Rect box = boxes[idx];

		bboxes.push_back(box);

		cv::Scalar color = cv::Scalar(0, 255, 0);
		drawPred(frame, class_ids[idx], confidences[idx], box.x, box.y,
		     	box.x + box.width, box.y + box.height, color);
    }

	return 0;
}

