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
Point2f * pMc;
Point2f mc;
int contourIndex;
int d;
int s;
}StrMonmentTag;
typedef struct StrCandicateTAG{
cv::Point2i  pt;
float MeanValue;
float StandDev;
}StrCandicateTag;
typedef struct StrAreaTAG{
	uint32_t lineIndex; //line index
	uint32_t mcIndex; //monmet index;
	//struct StrMonmentTAG  mc;
	uint32_t s; //surface of contour
	//uint32_t d; //diameter of contour
	uint32_t wws; // white width of score
	uint32_t bws; // black width of score
	float dls; // depth loss of score
	uint32_t doc; // degree of confidence
	Point    captruePoint; 
	int32_t  boxUp;
	int32_t  boxDown;
	float    Depth;
/*
    uint32_t x1;
    uint32_t x2; 
    uint32_t y1;
    uint32_t y2;
	uint32_t      AverCenterIndex; //center point of cadicate points       
	int32_t      AverCount;
	uint32_t     AverValue;
	cv::Point  AverPoint[100];  // aver point for calc
	*/
}strAreaTag;







#endif /* counter_hpp */
