//
//  counter.hpp
//  cvTest
//
//  Created by 覃永正 on 2017/10/29.
//  Copyright © 2017年 HFY. All rights reserved.
//

#ifndef counter_hpp
#define counter_hpp

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace std;
using namespace cv;
#define MAX(x,y)   (x>y?x:y)
#define MIN(x,y)   (x>y?y:x)

typedef struct StrAreaTAG{
    uint32_t s; //surface of contour
    uint32_t d; //diameter of contour
    uint32_t doc; // degree of confidence
    uint32_t x1;
    uint32_t x2; 
    uint32_t y1;
    uint32_t y2;
    cv::Point2i  moments;
    uint32_t index; //gContour index;
	Point    captruePoint; 
	uint32_t      AverCenterIndex; //center point of cadicate points       
	int32_t      AverCount;
	uint32_t     AverValue;
	cv::Point  AverPoint[100];  // aver point for calc
}strAreaTag;







#endif /* counter_hpp */
