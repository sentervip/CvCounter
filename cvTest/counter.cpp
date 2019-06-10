//
//  counter.cpp
//  cvTest
//.
//  Copyright © 2017年 HFY. All rights reserved.
//

#include "counter.hpp"
#include "fstream"
#include <iostream>
#include <string>
#include <strstream>
#include "math.h"

#define  FL_MIN_LENGTH   200
#define  FL_MIN_AREA     400
#define  DELTA           (6)
//#define  MIN_X           200
#define  MIN_Y           300
#define  MIN_CANDIDAT_DIAMETER    200
#define  MIN_CANDIDAT_AREA        1000

vector<vector<Point> > g_Contours;
vector<strAreaTag> stAreaTag,g_CandidateArea;
vector<strAreaTag> * pAreaTag = &stAreaTag;


string int2str(int index,int n1=0,int n2=0) {
    
    strstream ss;
    string s;
    ss<<index;
    if(n1){
    ss<<':';
    ss << n1;
    }
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
void AreaGetMax(strAreaTag * area)
{
    int i =0;
    while(i < g_Contours.at(area->index).size()){
        area->x1 = MIN(area->x1, g_Contours.at(area->index).at(i).x);
        area->x2 = MAX(area->x2, g_Contours.at(area->index).at(i).x);
        area->y1 = MIN(area->y1, g_Contours.at(area->index).at(i).y);
        area->y2 = MAX(area->y2, g_Contours.at(area->index).at(i).y);
        i++;
    }
    
}
int GetCandicatArea(strAreaTag * area,int w,int h)
{
    int Cx = w>>1;
    int Cy = h>>1;
    int delta = w / DELTA;
    
    if(area->s > MIN_CANDIDAT_AREA && area->d > MIN_CANDIDAT_DIAMETER){
        if( abs(area->moments.x - Cx) < delta && area->moments.y > MIN_Y ){
            g_CandidateArea.push_back(*area);
            return 1;
        }
    }
    return -1;
}
int main()
{
    int iRet;
    int mc_valid[300] = {1};
    RNG g_rng(12345);
    Mat g_cannyMat_output;
    vector<Vec4i> g_vHierarchy;
    Mat img = imread("/Users/tanyongzheng/Documents/CvCounter/cvTest/a7.png", 1);
    //imshow("OriImg", img);
    
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
    
    //s1 canny
    threshold(gray, bw, 0, 255, CV_THRESH_OTSU);
    imwrite("2bin.jpg", bw);


    Canny( bw, g_cannyMat_output, 20, 255, 3 );
    
    //s2 get contour
    findContours(bw, g_Contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    Mat markers = Mat::zeros(bw.size(), CV_32SC1);
    
    for (size_t i = 0; i < g_Contours.size(); i++)
        drawContours(markers, g_Contours, static_cast<int>(i), Scalar::all(static_cast<int>(i)+1), -1);
    
    circle(markers, Point(5, 5), 3, CV_RGB(255, 255, 255), -1);
    imwrite("3contours.jpg", markers);
    
    //s3 get center monments
    vector<Moments> mu(g_Contours.size() );
    for(unsigned int i = 0; i < g_Contours.size(); i++ )
    { mu[i] = moments( g_Contours[i], false ); }
    
    //  calc mc
    vector<Point2f> mc( g_Contours.size() );
    for( unsigned int i = 0; i < g_Contours.size(); i++ )
    {
        mc[i] = Point2i( static_cast<float>(mu[i].m10/mu[i].m00), static_cast<float>(mu[i].m01/mu[i].m00 )) ;
        //stAreaTag.at(i)->moments = &mc;
    }
    
    
    // s4 drawing contour and calc args of mc
    Mat drawing = Mat::zeros( g_cannyMat_output.size(), CV_8UC3 );
    Scalar color = CV_RGB(0,255,0);
    int sum = (int)g_Contours.size();
    int j =0;
    strAreaTag  area;
    for( unsigned int i = 0; i< g_Contours.size(); i++ )
    {
        area.d = arcLength( g_Contours[i], true );
        area.s = contourArea(g_Contours[i]);
        area.moments = mc.at(i);
        area.index = i;
        AreaGetMax(&area);
        stAreaTag.push_back(area);
        if(area.d < FL_MIN_LENGTH || area.s < FL_MIN_AREA){
            continue;
        }
        printf("[%d]s:%d , d: %d \n", i, mu[i].m00, area.s, area.d);
        
        iRet = GetCandicatArea(&area, drawing.cols,drawing.rows);
        if(iRet > 0){
            color = Scalar(0, 0, 255);
        }else{
            color = Scalar(0, 255,0);
        }
        drawContours( drawing, g_Contours, i, color, 2, 8, g_vHierarchy, 0, Point() );
        
        circle( drawing, mc[i], 3, Scalar(0,255,0), -1, 0.1f, 0 );
        putText(drawing,int2str(i,stAreaTag.at(i).s,stAreaTag.at(i).d),mc[i],CV_FONT_HERSHEY_DUPLEX,0.5f,Scalar(255,255,255));
        putText(drawing,int2str(area.moments.x, area.moments.y),Point2i(mc[i].x,mc[i].y-20),CV_FONT_HERSHEY_DUPLEX,  0.5f,Scalar(100,100,0));
        
    }
    
    putText(drawing,"Sum:",Point2f(0,50),CV_FONT_HERSHEY_DUPLEX,1.0f,color);
    putText(drawing,int2str(g_CandidateArea.size()),Point2f(80,50),CV_FONT_HERSHEY_SIMPLEX,0.8f,Scalar(255,255,255));
    imwrite( "4draw.jpg", drawing );
    return 0;
}
