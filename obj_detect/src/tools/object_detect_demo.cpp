#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video.hpp>

#include "obj_detector.h"

using namespace cv;
using namespace dnn;

static void usage(char *bin_name)
{
    printf("usage: %s -i image_file\n", bin_name);
   	printf("	-i: detect only one image file.\n");
    printf("example:\ndetect one img: %s -i ./001.jpg\n", bin_name);
}

int main(int argc, char** argv)
{
	std::string img_path;
	char ch;

	while ((ch = getopt(argc, argv, "hi:")) != EOF)
	{
		switch (ch)
		{
			case 'i':
				img_path = optarg;
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	if (img_path.empty())
	{
		usage(argv[0]);
		return -1;
	}

	cv::Mat frame;
	frame = cv::imread(img_path);
	if (frame.empty())
	{
		std::cout << "frame empty" << std::endl;
		return -1;
	}

	std::string model_path = "../model/yolov3.weights";
	std::string cfg_path = "../model/yolov3.cfg";

	ObjDetector *obj_detector = new ObjDetector(model_path, cfg_path);
	if (obj_detector == NULL)
	{
		std::cout << "object detector init fail" << std::endl;
		return -1;
	}

	std::vector<cv::Point2d> points;
	points.clear();
	//points.push_back(cv::Point(frame.rows / 3, 0));
	//points.push_back(cv::Point(frame.rows / 3, frame.rows));
	//points.push_back(cv::Point(frame.rows / 4 * 3, 0));
	//points.push_back(cv::Point(frame.rows / 4 * 3, frame.rows));
	points.push_back(cv::Point2d(0.8, 0.0));
	points.push_back(cv::Point2d(0.8, 1.0));

	bool result = false;
	int ret = obj_detector->detect(frame, points, 1, true, result);
	if (ret < 0)
	{
		std::cout << "object detect failed" << std::endl;
		delete obj_detector;
		return -1;
	}

	std::cout << "detect result: " << result << std::endl;

    return 0;
}

