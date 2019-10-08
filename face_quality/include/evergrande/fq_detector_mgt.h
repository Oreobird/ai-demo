#ifndef __FQ_DETECTOR_MGT_H__
#define __FQ_DETECTOR_MGT_H__

#include "base_common.h"
#include "base_singleton_t.h"
#include "base_thread_mutex.h"
#include "base_rw_thread_mutex.h"


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/dnn.hpp>

#include "fq_common.h"
#include "fq_detector.h"
#include "fq_emotion.h"
#include "fq_occlusion.h"
#include "fq_quality.h"

//typedef unsigned long long uint64;
USING_NS_BASE;
using namespace std;
using namespace cv;

typedef struct _detector
{
	Detector *face_detector;
	OcclusionDetector *occ_detector;
	EmotionRecognizer *emt_recognizer;
	Quality *quality;

    bool busy;
} detector_t;


class Detector_Mgt
{
public:
	Detector_Mgt();

	~Detector_Mgt();

	int init(std::string &model_dir);

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
