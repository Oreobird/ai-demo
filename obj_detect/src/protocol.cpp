#include "protocol.h"
#include "base_time.h"
#include "base_string.h"
#include "base_convert.h"
#include "base_logger.h"
#include "base_base64.h"
#include "common.h"

#include "rapidjson/document.h"        // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h"    // for stringify JSON
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

extern base::Logger g_logger;

#define GET_JSON_STRING_NODE(father, name, result, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        const rapidjson::Value &node = (father)[name.c_str()]; \
        if(!node.IsString()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't string, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't string type"); \
            return -1; \
        } \
        result = node.GetString(); \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}

#define GET_JSON_ARRAY_NODE(father, name, node, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        node = (father)[name.c_str()]; \
        if(!node.IsArray()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't array type, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't array type"); \
            return -1; \
        } \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}

#define GET_JSON_UNSIGNED_INTEGER_NODE(father, name, result, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        const rapidjson::Value &node = (father)[name.c_str()]; \
        if(!node.IsUint()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't unsigned integer, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't unsigned integer type"); \
            return -1; \
        } \
        result = node.GetUint(); \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}

#define GET_JSON_BOOL_NODE(father, name, result, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        const rapidjson::Value &node = (father)[name.c_str()]; \
        if(!node.IsBool()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't bool, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't bool type"); \
            return -1; \
        } \
        result = node.GetBool(); \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}


#define GET_JSON_DOUBLE_NODE(father, name, result, errInfo, ifNeed) \
{\
    errInfo = ""; \
    if((father).HasMember(name.c_str())) \
    { \
        const rapidjson::Value &node = (father)[name.c_str()]; \
        if(!node.IsDouble()) \
        { \
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, %s isn't double, req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
            errInfo.append("it is invalid req, ").append(name).append(" isn't double type"); \
            return -1; \
        } \
        result = node.GetDouble(); \
    } \
    else if(ifNeed) \
    { \
        XCP_LOGGER_ERROR(&g_logger, "cannot find %s of req(%u):%s\n", name.c_str(), req.size(), req.c_str()); \
        errInfo.append("it is invalid req, cannot find ").append(name); \
        return -1; \
    } \
}


int XProtocol::req_parse(const std::string &req,
								std::string &img_data,
								std::string &video_path,
						 		std::vector<cv::Point2d> &pts_ratio,
						 		std::string &err_info)
{
    std::string nodeName;
    rapidjson::Document root;
    root.Parse(req.c_str());
    if (root.HasParseError() || !root.IsObject())
    {
        XCP_LOGGER_ERROR(&g_logger, "rapid json parse failed, req:%s\n", req.c_str());
        err_info = "it is invalid json req.";
        return -1;
    }

	//line_points
    nodeName = "line_points";
    rapidjson::Value list;
    GET_JSON_ARRAY_NODE(root, nodeName, list, err_info, true);

    for (Value::ConstValueIterator iter = list.Begin(); iter != list.End(); ++iter)
    {
        if(!iter->IsObject())
        {
            XCP_LOGGER_INFO(&g_logger, "it is invalid req, list element isn't json object, req(%u):%s\n", req.size(), req.c_str());
            return -1;
        }

        cv::Point2d point_ratio;
        nodeName = "x";
        GET_JSON_DOUBLE_NODE(*iter, nodeName, point_ratio.x, err_info, true);
        nodeName = "y";
        GET_JSON_DOUBLE_NODE(*iter, nodeName, point_ratio.y, err_info, false);
        pts_ratio.push_back(point_ratio);
    }

    //img_data
    nodeName = "img_data";
    GET_JSON_STRING_NODE(root, nodeName, img_data, err_info, false);

    //video_path
    nodeName = "video_path";
    GET_JSON_STRING_NODE(root, nodeName, video_path, err_info, false);

	return 0;
}

std::string XProtocol::gen_detect_result(const std::string &img_str, const bool alarm, const int err_code)
{
	std::ostringstream json_str;
	json_str << "{\"err_code\":" << err_code
			<< ",\"alarm\":" << alarm
			<< ",\"img_data\":\"" << img_str
			<< "\"}";

	return json_str.str();
}

std::string XProtocol::gen_video_detect_result(const std::string &video_path, const int err_code)
{
	std::ostringstream json_str;
	json_str << "{\"err_code\":" << err_code
			<< ",\"video_path\":\"" << video_path
			<< "\"}";

	return json_str.str();
}

std::string XProtocol::gen_video_detect_progress(const int progress, const int err_code)
{
	std::ostringstream json_str;
	json_str << "{\"err_code\":" << err_code
			<< ",\"progress\":\"" << progress
			<< "%\"}";

	return json_str.str();
}


std::string XProtocol::gen_failed_result(const int err_code, const std::string &err_msg)
{
	std::ostringstream json_str;
	json_str << "{\"err_code\":" << err_code
			<< ",\"err_msg\":\"" << err_msg
			<< "\"}";

	return json_str.str();
}

int XProtocol::get_specific_params(const std::string &req, const std::string &paramName, std::string &result, std::string &err_info)
{
    rapidjson::Document root;
    root.Parse(req.c_str());
    if (root.HasParseError() || !root.IsObject())
    {
        XCP_LOGGER_ERROR(&g_logger, "rapid json parse failed, req:%s\n", req.c_str());
        err_info = "it is invalid json req.";
        return -1;
    }

    GET_JSON_STRING_NODE(root, paramName, result, err_info, true);
    XCP_LOGGER_INFO(&g_logger, "get param %s success, result:%s\n", paramName.c_str(), result.c_str());

    return 0;
}

int XProtocol::get_specific_params(const std::string &req, const std::string &paramName, bool &result, std::string &err_info)
{
    rapidjson::Document root;
    root.Parse(req.c_str());
    if (root.HasParseError() || !root.IsObject())
    {
        XCP_LOGGER_ERROR(&g_logger, "rapid json parse failed, req:%s\n", req.c_str());
        err_info = "it is invalid json req.";
        return -1;
    }

    GET_JSON_BOOL_NODE(root, paramName, result, err_info, true);
    XCP_LOGGER_INFO(&g_logger, "get param %s success, result:%u\n", paramName.c_str(), result);

    return 0;
}

int XProtocol::get_specific_params(const std::string &req, const std::string &paramName, unsigned int &result, std::string &err_info)
{
    rapidjson::Document root;
    root.Parse(req.c_str());
    if (root.HasParseError() || !root.IsObject())
    {
        XCP_LOGGER_ERROR(&g_logger, "rapid json parse failed, req:%s\n", req.c_str());
        err_info = "it is invalid json req.";
        return -1;
    }

    GET_JSON_UNSIGNED_INTEGER_NODE(root, paramName, result, err_info, true);
    XCP_LOGGER_INFO(&g_logger, "get param %s success, result:%u\n", paramName.c_str(), result);

    return 0;
}
