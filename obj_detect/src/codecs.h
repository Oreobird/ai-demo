#ifndef __CODEC_H__
#define __CODEC_H__

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <vector>
#include <string>


using namespace std;
using namespace cv;


class MatCodecs
{
public:
	MatCodecs();

	cv::Mat base64_to_mat(const std::string &imageBase64);
	std::string mat_to_base64(const cv::Mat &img);

private:
    std::string base64_encode(uchar const* bytes_to_encode, unsigned int in_len);
	std::string base64_decode(std::string const& encoded_string);
};

#endif

