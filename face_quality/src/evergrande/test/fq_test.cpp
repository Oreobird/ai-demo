#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>
#include <dirent.h>
#include <stdlib.h>
#include <libgen.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/dnn.hpp>

#include "fq_common.h"
#include "fq_quality.h"
#include "fq_timing.h"
#include "fq_emotion.h"
#include "fq_occlusion.h"
#include "fq_detector.h"
#include "fq_json.h"

using namespace std;
using namespace cv;

std::string model_dir = "/../model/";
std::string img_file;
std::string result_img_path;
std::string img_dir;
std::string crop_face_path;
std::string quality_path;

static void usage(char *bin_name)
{
    printf("usage: %s -i image_file -j json_file -o result.jpg -t type\n", bin_name);
   	printf("	-i: detect only one image file.\n");
   	printf("	-d: detect all images in dir\n");
   	printf("	-o: output detect result image file path.\n");
   	printf("	-c: crop face image save path.\n");
   	printf("	-q: quality output file.\n");
    printf("example:\ndetect one img: %s -i ./001.jpg -q ./quality.txt\n", bin_name);
    printf("detect imgs in dir: %s -d ./data -q ./quality.txt\n", bin_name);
    printf("crop face: %s -d ./data -c ./data/face/\n", bin_name);
}

void handle_img(Mat img, Detector *detector, Quality *quality, ofstream &fp, std::string name)
{
	double start = now();
	double detect_time;
	double landmark_time;
	double quality_time;
	double draw_save_time;

	std::vector<landmark_t> mark;

	int face_num = detector->face_detect(img);

	#ifdef CAL_TIME
	detect_time = calcElapsed(start, now());
	printf("detect_time: %d ms.\n", (int)(detect_time * 1000));
	start = now();
	#endif

	detector->landmark_detect(img, &mark);

	#ifdef CAL_TIME
	landmark_time = calcElapsed(start, now());
	printf("landmark_time: %d ms.\n", (int)(landmark_time * 1000));
	start = now();
	#endif

	if (face_num == 1)
	{
		if (!crop_face_path.empty())
		{
			Mat crop_img = img(mark[0].emotion_box);
			Mat input_img;
			cv::resize(crop_img, input_img, cv::Size(120, 120));
			//cv::imshow("face", input_img);
			//cv::waitKey(0);
			std::string face_path = crop_face_path + basename((char *)name.c_str());
			cout << face_path << endl;
			cv::imwrite(face_path, input_img);
			return;
		}


		float occ_prob;
		OcclusionDetector occ_detector(model_dir);
		occ_detector.detect(img, mark[0].emotion_box, 96, &occ_prob);
		quality->set("occlusion", occ_prob);
		//occ_detector.draw(img, cv::Point(10, 60), cv::Scalar(0, 255, 0));

		float emt_prob;
		EmotionRecognizer emt_recognizer(model_dir);
		emt_recognizer.recognize(img, mark[0].emotion_box, 48, &emt_prob);
		quality->set("emotion", emt_prob);
		//emt_recognizer.draw(img, cv::Point(10, 30), cv::Scalar(0, 0, 255));

		quality->cal_all_scores(img, &mark[0]);

		quality->set("pass", quality->is_pass()?1.:0.);

		if (fp)
		{
			//char name[64] = {0};
			//snprintf(name, sizeof(name), "%d.jpg", i);
			fp << name <<","<<quality->at("pass")<<","
			    <<quality->at("size_score")<<","<<quality->at("position_score")<<","
				<<quality->at("yaw_score")<<","<<quality->at("pitch_score")<<","
				<<quality->at("roll_score")<<","<<quality->at("contrast_score")<<","
				<<quality->at("clarity_score")<<","<<quality->at("brightness_score")<<","
				<<emt_recognizer.at("abnormal")<<","<<emt_recognizer.at("normal")<<","
				<<quality->at("occlusion")<<","
				<<occ_detector.at("left-eye")<<","<<occ_detector.at("right-eye")<<","
				<<occ_detector.at("nose")<<","<<occ_detector.at("mouth")<<","
				<<occ_detector.at("chin")<<endl;
		}

	}
	else if (face_num == 0)
	{
		fp << name << "," << "no face detect" << endl;
	}
	else
	{
		fp << name << "," << "multi face detect" << endl;
	}

	#ifdef CAL_TIME
	quality_time = calcElapsed(start, now());
	printf("quality_time: %d ms.\n", (int)(quality_time * 1000));
	start = now();
	#endif

	if (!result_img_path.empty())
	{
		detector->landmark_draw(img, cv::Scalar(0, 0, 255), false, result_img_path);
	}

	#ifdef CAL_TIME
	draw_save_time = calcElapsed(start, now());
	printf("draw_save_time: %d ms.\n", (int)(draw_save_time * 1000));
	#endif
}

typedef void (*file_fn)(Detector *detector, Quality *quality, std::string file_path, ofstream &fp);

void walk_dir(Detector *detector, Quality *quality, std::string dir, ofstream &fp, file_fn cb)
{
	struct stat fs;
	lstat(dir.c_str(), &fs);

	if (!S_ISDIR(fs.st_mode))
	{
		cout << "dir is not a valid dir"<<endl;
		return;
	}

	struct dirent *filename;
	DIR *pdir = NULL;

	pdir = opendir(dir.c_str());
	if (!pdir)
	{
		cout << "can not open dir" << dir << endl;
		return;
	}

	cout << "Open dir:"<<dir<<endl;

	char tmp_file[256] = {0};

	while ((filename = readdir(pdir)) != NULL)
	{
		if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0)
		{
			continue;

}

		snprintf(tmp_file, sizeof(tmp_file), "%s/%s", dir.c_str(), filename->d_name);
		struct stat fs;
		lstat(tmp_file, &fs);

		if (S_ISDIR(fs.st_mode))
		{
 			cout << filename->d_name << endl;
			walk_dir(detector, quality, tmp_file, fp, cb);
		}
 		else
 		{
 			printf("%s\n", tmp_file);
 			cb(detector, quality, tmp_file, fp);
 		}
 	}

	closedir(pdir);
}

void detect_img(Detector *detector, Quality *quality, std::string file_path, ofstream &fp)
{
	cv::Mat img = imread(file_path);
	if (img.empty())
	{
		fa_dbg("imread failed\n");
		return;
	}

	handle_img(img, detector, quality, fp, file_path);
}

int main(int argc, char** argv)
{
	char prog_path[256] = {0};
	char ch;

	char *p = strrchr(argv[0], '/');
	strncpy(prog_path, argv[0], p - argv[0]);
	model_dir = prog_path + model_dir;

	while ((ch = getopt(argc, argv, "hi:o:d:c:q:")) != EOF)
	{
		switch (ch)
		{
			case 'i':
				img_file = optarg;
				break;
			case 'o':
				result_img_path = optarg;
				break;
			case 'd':
				img_dir = optarg;
				break;
			case 'c':
				crop_face_path = optarg;
				break;
			case 'q':
				quality_path = optarg;
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			default:
				break;
		}
	}

	if (img_file.empty() && img_dir.empty())
	{
		usage(argv[0]);
		return -1;
	}

	if (quality_path.empty())
	{
		char default_dir[256] = {0};
		snprintf(default_dir, sizeof(default_dir), "%s/../data/quality.txt", prog_path);
		quality_path = default_dir;
	}

	ofstream fp(quality_path);
	if (!fp)
	{
		fa_dbg("fp create failed\n");
		return -1;
	}

	fp << "file_name,pass,size,position,yaw,pitch,roll,constrast,clarity,bright,emt_abnormal,emt_normal,occlusion,left-eye,right-eye,nose,mouth,chin" << endl;

	DlibLandmarkDetector *dlib_lm_detector = new DlibLandmarkDetector(model_dir);
	Detector detector(dlib_lm_detector, model_dir);
	Quality quality(model_dir);

	if (!img_dir.empty())
	{
		walk_dir(&detector, &quality, img_dir, fp, detect_img);
	}
	else if (!img_file.empty())
	{
		detect_img(&detector, &quality, img_file, fp);
	}

	fp.close();

	return 0;
}
