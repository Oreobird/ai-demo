#include <stdio.h>
#include <math.h>
#include <iostream>
#include <map>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "fq_common.h"
#include "fq_codecs.h"

using namespace cv;
using namespace std;

static void usage(char *bin_name)
{
    printf("usage: %s -i base64_file -o result.jpg\n", bin_name);
   	printf("	-i: detect only one image file.\n");
   	printf("	-o: output result image file path.\n");
    printf("example:\ndetect one img: %s -i ./001.jpg -o ./haha.jpg\n", bin_name);
}

void readTxt(const std::string &file, std::string &content)
{
    ifstream infile;

    infile.open(file.data());

    assert(infile.is_open());

    getline(infile, content);

    infile.close();
}

int main(int argc, char** argv)
{
	char ch;
	std::string base64_file;
	std::string img_path;

	while ((ch = getopt(argc, argv, "hi:o:")) != EOF)
	{
		switch (ch)
		{
			case 'i':
				base64_file = optarg;
				break;
			case 'o':
				img_path = optarg;
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	std::string base64_data;

	readTxt(base64_file, base64_data);

	MatCodecs mat_codecs;
	printf("base64 data len:%ld\n", base64_data.size());

	Mat img = mat_codecs.base64_to_mat(base64_data);
	printf("img width:%d, height:%d\n", img.cols, img.rows);

	cv::imwrite(img_path, img);

	return 0;
}
