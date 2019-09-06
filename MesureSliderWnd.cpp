#include "counter.hpp"
#include "fstream"
#include <iostream>
#include <string>
#include <strstream>
#include "math.h"
#include <stdlib.h>
#include <algorithm> 
#include <iostream>
#include <deque>

#define  WIN_W    20
#define  WIN_H    10
StrCandicateTag Candicate;
vector<StrCandicateTag> g_StrCandicateTag;
vector<StrCandicateTag> * pStrCandicateTag = &g_StrCandicateTag;

int SortCmp(StrCandicateTag a1, StrCandicateTag a2)
{
	return a1.StandDev < a2.StandDev;
}

//用deque<Point> 描述曲线
//随机取色画曲线
Scalar random_color(RNG& _rng)
{
	int icolor = (unsigned)_rng;
	return Scalar(icolor & 0xFF, (icolor >> 8) & 0xFF, (icolor >> 16) & 0xFF);
}
int SliderWin(Mat src,cv::Point2i pt, int w,int h, StrCandicateTag *pData)
{	
	cv::Scalar     mean;  
    cv::Scalar     dev; 
	cv::Rect2i rect(pt.x, pt.y, w,h);
	if(src.rows < rect.y || src.cols < rect.x){
		printf("roi invalid\n");
	    return -1;
	}
	Mat Roi = src(rect);
    cv::meanStdDev ( Roi, mean, dev );  
    pData->MeanValue = mean.val[0];  
    pData->StandDev = dev.val[0];  
    std::cout << pData->MeanValue << ",\t" << pData->StandDev; 
	return 0;
}
int main()
{
    int i,iRet = 0;
	char dist[40];
    int mc_valid[300] = {1};
    RNG g_rng(12345);
    Mat g_cannyMat_output;
    vector<Vec4i> g_vHierarchy;
    Mat img = imread("D:/prj/z1/src/CvCounter/win32/2.jpg", 1);

    int width = img.cols;
    int height = img.rows;
    Mat gray = Mat::zeros(img.size(),CV_8UC1);
    for ( i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int pix = img.at<Vec3b>(i, j)[0] + img.at<Vec3b>(i, j)[2] - img.at<Vec3b>(i, j)[1];
            if (pix > 255)
                pix = 255;
            if (pix < 0)
                pix = 0;
            gray.at<uchar>(i, j) = (uchar)pix;
        }
    }
    imwrite("1gray.jpg",gray);
    Mat DestRGB;// = Mat::zeros( gray.size(), CV_8UC3 );
    Scalar color = CV_RGB(0,255,0);
	
	for(i=WIN_H; i< (gray.rows - WIN_H); i++)
    {
		printf("[%d]\n", i);
		Candicate.pt= cv::Point2i(gray.cols/2, i);
		iRet = SliderWin(gray,cv::Point2i(gray.cols/2, i), WIN_W, WIN_H,&Candicate);
		cv::cvtColor(gray,DestRGB,COLOR_GRAY2RGB);
		cv::Rect2i rect(gray.cols/2, i, WIN_W,WIN_H);
        cv::rectangle(DestRGB,rect,Scalar(0, 0, 0xff),2);  
		memset(dist,0,sizeof(dist));
		sprintf(dist,"   %d mm", (rect.y+WIN_H)/2+i);
		putText(DestRGB,dist,Point2d(gray.cols/2,i/2),
		                 CV_FONT_HERSHEY_DUPLEX,0.4f,Scalar(0,0,255));
		cv::arrowedLine(DestRGB,Point(gray.cols/2+WIN_W/2,0),Point(rect.x+WIN_W/2,(rect.y+WIN_H/2)),Scalar(0,0,255),1);
		pStrCandicateTag->push_back(Candicate);
		cv::imshow("dest",DestRGB);
		//waitKey();
    }
    std::sort(g_StrCandicateTag.begin(),g_StrCandicateTag.end(),SortCmp);
	printf("depth:%d", g_StrCandicateTag.at(g_StrCandicateTag.size()-1).pt.y+WIN_H+7);
	waitKey();
    return i;
}
