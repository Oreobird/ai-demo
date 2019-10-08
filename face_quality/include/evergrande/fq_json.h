#ifndef __FQ_JSON_H__
#define __FQ_JSON_H__

#include <cstdint>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <json/json.h>

#include <iomanip>

#include "fq_quality.h"
#include "fq_detector.h"

using namespace std;

class EvgJson
{
public:
    EvgJson(){};
    EvgJson(std::string &json_path);
    int gen_exception_json(int face_num, std::string &img_data, std::string &json_str);
    int gen_face_json(landmark_t *mark, Quality *qof, std::string &img_data, std::string &json_str);
private:
    std::string m_json_path;
};

#endif
