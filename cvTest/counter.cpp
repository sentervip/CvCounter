//
//  counter.cpp
//  cvTest
//.
//  Copyright © 2017年 HFY. All rights reserved.
//

#include "counter.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "fstream"
#include <iostream>
#include <string>
#include <strstream>
#include <vector>
#include "math.h"

using namespace cv;
using namespace std;

#define  FL_MIN_LENGTH   200
#define  FL_MIN_AREA     400
string int2str(int n,int n2=0) {
    
    strstream ss;
    string s;
    ss << n;
    if(n2){
        ss<<',';
        ss<<n2;
    }
    ss >> s;
    
    return s;
}
int sort_point(vector<Point2f> mc, int * valid, Mat &drawing)
{
    if(mc.size() <2 )
        return -1;
    
    Point2f pt_tmp;
    int i =0,j=0;
    for(vector<cv::Point2f>::iterator iter = mc.begin(); iter != mc.end(); ++iter){
        if(i>0) //s1: repeate
            if( abs(iter->x - pt_tmp.x) < 2 || abs(iter->y - pt_tmp.y) < 2 ){
                mc.erase(iter);
                cout<<"erase i: "<<i<<std::endl;
                putText(drawing,int2str(i),pt_tmp,CV_FONT_HERSHEY_DUPLEX,0.8f,CV_RGB(255,0,0));
                valid[i] = 0;
                j++;
            }
        i++;
        pt_tmp = *iter;
        
    }
    
    return (i - j+1);
}

int main()
{
    int mc_valid[300] = {1};
    RNG g_rng(12345);
    Mat g_cannyMat_output;
    vector<Vec4i> g_vHierarchy;
    Mat img = imread("/Users/tanyongzheng/Documents/svn-cvTest/cvTest/a7.png", 1);
    imshow("OriImg", img);
    
    GaussianBlur(img, img, Size(5, 5), 0.5);
    
    int width = img.cols;
    int height = img.rows;
    Mat gray = Mat::zeros(img.size(),CV_8UC1);
    for (int i = 0; i < height; i++)
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
    Mat bw;
    threshold(gray, bw, 0, 255, CV_THRESH_OTSU);
    imwrite("2bin.jpg", bw);


    Canny( bw, g_cannyMat_output, 20, 255, 3 );
    vector<vector<Point> > g_vContours,gSortPoint2;
    
    findContours(bw, g_vContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);    
    Mat markers = Mat::zeros(bw.size(), CV_32SC1);
    
    for (size_t i = 0; i < g_vContours.size(); i++)
        drawContours(markers, g_vContours, static_cast<int>(i), Scalar::all(static_cast<int>(i)+1), -1);
    
    circle(markers, Point(5, 5), 3, CV_RGB(255, 255, 255), -1);
    imwrite("3contours.jpg", markers);
    
    // 计算矩
    vector<Moments> mu(g_vContours.size() );
    for(unsigned int i = 0; i < g_vContours.size(); i++ )
    { mu[i] = moments( g_vContours[i], false ); }
    
    //  计算中心矩
    vector<Point2f> mc( g_vContours.size() );
    for( unsigned int i = 0; i < g_vContours.size(); i++ )
    {
        mc[i] = Point2f( static_cast<float>(mu[i].m10/mu[i].m00), static_cast<float>(mu[i].m01/mu[i].m00 ));
    }
    
    // 绘制轮廓
    Mat drawing = Mat::zeros( g_cannyMat_output.size(), CV_8UC3 );
    Scalar color = CV_RGB(255,255,255);
    memset(mc_valid, 1, sizeof(mc_valid));
    int sum = g_vContours.size();
    sort_point(mc, mc_valid,drawing);
    int j =0;
    for( unsigned int i = 0; i< g_vContours.size(); i++ )
    {
        int len = arcLength( g_vContours[i], true );
        int area = contourArea(g_vContours[i]);
        
        if(len < FL_MIN_LENGTH || area < FL_MIN_AREA){
            //mc.erase(mc[i]);
            continue;
        }
        printf("通过m00计算出轮廓[%d]的面积: (M_00) = %.2f \n OpenCV函数计算出的面积=%d , 长度: %d \n", i, mu[i].m00, area, len);
        j++;
        gSortPoint2.push_back(g_vContours[i]);
        drawContours( drawing, g_vContours, i, Scalar(0,0,255), 2, 8, g_vHierarchy, 0, Point() );//绘制外层和内层轮廓
        circle( drawing, mc[i], 3, Scalar(0,255,0), -1, 0.1f, 0 );//绘制圆
        cout<<"int2str: "<<int2str(i)<<" mc: "<<mc[i]<<endl;
        putText(drawing,int2str(area,len),mc[i],CV_FONT_HERSHEY_DUPLEX,0.8f,color);
        //}
    }
    sum = j;
    putText(drawing,"Sum:",Point2f(0,50),CV_FONT_HERSHEY_DUPLEX,1.0f,color);
    putText(drawing,int2str(sum),Point2f(100,50),CV_FONT_HERSHEY_DUPLEX,1.0f,color);
    imwrite( "4draw.jpg", drawing );
    return 0;
}
