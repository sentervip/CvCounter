#ifndef counter_hpp
#define counter_hpp

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace std;
using namespace cv;

//#define MAX(x,y)   (x>y?x:y)
//#define MIN(x,y)   (x>y?y:x)
//picture detect
#define EINVALID_ARG     (-1)
#define ENULL_WAVE       (-2)
#define ELOW_WAVE        (-3)
#define EOVER_PRESSED_WAVE    (-4)
#define ELIMIT           (-5)

#define LIMIT_WAIST   300
#define LIMIT_ARM     200
enum BODY_PART{
	PART_WAIST = 0,
	PART_FACE,
	PART_ARM,
	PART_THIGH,
    PART_CHEST,
    PART_BELLY,
};
typedef struct StrMonmentTAG{
int d;
int s;
int contourIndex;
unsigned int AveYAxis; 
Point2f * pMc;
Point2f mc;
vector<Point> PtsEdges;
}StrMonmentTag;
typedef struct StrCandicateTAG{
cv::Point2i  pt;
float MeanValue;
float StandDev;
}StrCandicateTag;
typedef struct StrAreaTAG{
	uint32_t McIndex; //monmet index;
	float dls; // depth loss of score
	float doc; // degree of confidence
}strAreaTag;







#endif /* counter_hpp */
