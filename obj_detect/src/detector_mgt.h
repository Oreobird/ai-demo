#ifndef __DETECTOR_MGT_H__
#define __DETECTOR_MGT_H__

#include "base_common.h"
#include "base_singleton_t.h"
#include "base_thread_mutex.h"
#include "base_rw_thread_mutex.h"
#include "base_condition.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/dnn.hpp>

#include "common.h"
#include "obj_detector.h"

//typedef unsigned long long uint64;
USING_NS_BASE;
using namespace std;
using namespace cv;

typedef struct _detector
{
	ObjDetector *obj_detector;
    bool busy;
} detector_t;


class Detector_Mgt
{
public:
	Detector_Mgt();

	~Detector_Mgt();

	int init(const std::string &model_dir);

    int get_detector(detector_t **detector);

    int put_detector(detector_t **detector);


private:
	Thread_Mutex _mutex;
	std::vector<detector_t*> m_detectors;
    unsigned short m_detector_size;

    unsigned short m_free_detector_num;
    Condition _cond;
};

#define PSGT_Detector_Mgt Singleton_T<Detector_Mgt>::getInstance()

#endif
