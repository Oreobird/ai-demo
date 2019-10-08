#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "common.h"

#include <opencv2/core.hpp>


class XProtocol
{
public:
    static int req_parse(const std::string &req, std::string &img_data,
						std::string &video_path, std::vector<cv::Point2d> &pts_ratio,
						std::string &err_info);
    static std::string gen_detect_result(const std::string &img_str, const bool alarm, const int err_code);
    static std::string gen_video_detect_result(const std::string &video_path, const int err_code);
    static std::string gen_video_detect_progress(const int progress, const int err_code);

    static std::string gen_failed_result(const int err_code, const std::string &err_msg);

    static int get_specific_params(const std::string &req, const std::string &paramName, std::string &result, std::string &err_info);
    static int get_specific_params(const std::string &req, const std::string &paramName, bool &result, std::string &err_info);
    static int get_specific_params(const std::string &req, const std::string &paramName, unsigned int &result, std::string &err_info);
};


#endif

