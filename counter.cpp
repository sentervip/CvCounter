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
#include <stdlib.h>
#include<algorithm> 
#include <vector>

#define  IMG_WIDTH      160
#define  IMG_HEIGH      480

#define  FL_MIN_LENGTH   (IMG_WIDTH*0.6)
#define  FL_MIN_AREA     400
#define  DELTA           (6)  // contour error   1/6 *w
#define  DELTA_POINT     (10)  // point error  1/10 *w
#define  SUM_CALC_POINT   (100)
//#define  MIN_X           200
#define  MIN_Y           300
#define  MIN_CANDIDAT_DIAMETER    200
#define  MIN_CANDIDAT_AREA        700
#define  MIN_DISTANCE_CLUSTER     20
#define  MIN_IMG_Y                MIN_DISTANCE_CLUSTER
#define  CALC_WIDTH_UP            1
#define  CALC_WIDTH_DOWN          2

vector<StrMonmentTag> g_MonmentTag;  //momentes struct
vector<vector<Point> > g_Contours;
vector<strAreaTag> g_AllArea,g_CandidateArea; //all area and area of lines
vector<strAreaTag> * pAreaTag = &g_AllArea;


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
void AreaGetMax(strAreaTag * area, int w, int h)
{
    int i =0;
	area->x1 = w;
	area->y1 = h;
    while(i < g_Contours.at(area->mcIndex).size()){
        area->x1 = MIN(area->x1, g_Contours.at(area->mcIndex).at(i).x);
        area->x2 = MAX(area->x2, g_Contours.at(area->mcIndex).at(i).x);
        area->y1 = MIN(area->y1, g_Contours.at(area->mcIndex).at(i).y);
        area->y2 = MAX(area->y2, g_Contours.at(area->mcIndex).at(i).y);
        i++;
    }
    
}

int SortCmp(StrMonmentTag a1, StrMonmentTag a2)
{
	return a1.s > a2.s;
}
int SortCmpUp(int a1, int a2)
{
	return a1 < a2;
}

int GetDistance(CvPoint pointO,CvPoint pointA )  
{  
	int distance;  
	distance = powf((pointO.x - pointA.x),2) + powf((pointO.y - pointA.y),2);  
	distance = sqrtf(distance);  
	return distance;  
}

int PointClustering(vector<Vec2f> _pts, vector<int> & out)
{
	int i,capture;
    vector<int>::iterator it;
	vector<int> pts;

	//get pts and sort
	for(i=0;i< _pts.size(); i++){
		pts.push_back(cvRound(_pts[i][0]) );
	}
	std::sort(pts.begin(), pts.end(),SortCmpUp);

	//clustering
	while(pts.size()>0){
		if(pts.size() == 1){
		    out.push_back(pts.at(0));
			break;
		}else{
			capture= pts.at(0);
			out.push_back(capture);
			pts.erase(pts.begin());
			for(it=pts.begin();it != pts.end();){
				if(abs(capture - *it) < MIN_DISTANCE_CLUSTER){
					it = pts.erase(it);
				}else{
					it++;
				}
			}
		}


	}
	return 0;
}
int Match2Moment(vector<StrMonmentTag> MomentTag,  int lineY)
{
    vector<StrMonmentTag>::iterator it;
	int index = 0;
	int i=0;
	int Min = 500;

	for(it=MomentTag.begin();it != MomentTag.end(); it++){
		if(abs(it->mc.y-lineY) <Min && it->s > (MomentTag.at(0).s/2) && (it->d > FL_MIN_LENGTH) ){
			Min = it->mc.y;
			index = i;
		}
		i++;
	}
	return index;
}
int CalcWidth(cv::Mat &bin,Point2i pt, int len, int UpOrDown)
{
	int aver =0;
	int sum =0;
	int width = 0;

	
	while(aver < 0.7*len){
		for(int i=-len/2;i<len/2; i++){
			sum += bin.at<int>(pt.y,IMG_WIDTH/2+i);
		}
		aver = sum/len;
		if(UpOrDown == CALC_WIDTH_UP){
		    if(pt.y >MIN_IMG_Y){
				pt.y--;
				width++;
			}else{
				printf("error, %s,invalid img1",__FUNCTION__);
				width = -1;
				break;
			}
	   }else{
		   if(pt.y >MIN_IMG_Y){
			   pt.y++;
			   width++;
		   }else{
			   printf("error, %s,invalid img2",__FUNCTION__);
			   width = -1;
			   break;
		   }
		}
	}
	return width;
}
int main()
{
    int i,j,iRet = 0;
	Point pt1, pt2;
    int mc_valid[300] = {1};
    RNG g_rng(12345);
    Mat g_cannyMat_output;
    vector<Vec4i> g_vHierarchy;
	vector<int> g_lineY;
    Mat m_origin = imread("D:/prj/z1/src/img/0904_ai/4.png", 1.0);
    
    //s1 preprocess
    GaussianBlur(m_origin, m_origin, Size(15, 15), 1.0);   
    int width = m_origin.cols;
    int height = m_origin.rows;
    Mat m_gray = Mat::zeros(m_origin.size(),CV_8UC1);
    for ( i = 0; i < height; i++)
    {
        for ( j = 0; j < width; j++)
        {
            int pix = m_origin.at<Vec3b>(i, j)[0] + m_origin.at<Vec3b>(i, j)[2] - m_origin.at<Vec3b>(i, j)[1];
            if (pix > 255)
                pix = 255;
            if (pix < 0)
                pix = 0;
            m_gray.at<uchar>(i, j) = (uchar)pix;
        }
    }
	imwrite("gray.jpg", m_gray);

    //morphology
	int g_nElementShape = MORPH_RECT;
	Mat element = getStructuringElement(g_nElementShape, Size(5, 5) );
	morphologyEx(m_gray, m_gray, CV_MOP_ERODE, element);  imwrite("1erode.jpg",m_gray);
	element = getStructuringElement(g_nElementShape, Size(7, 7) );
	morphologyEx(m_gray, m_gray, CV_MOP_DILATE, element);
    imwrite("2dilate.jpg",m_gray);
    Mat m_bin;
    
    //Bin
    threshold(m_gray, m_bin, 0, 255, CV_THRESH_OTSU);
    imwrite("3bin.jpg", m_bin);
	
	//lines
	Mat m_line(m_bin.size(),CV_8UC1);

	#if 1
	vector<Vec2f> lines; 
	HoughLines(m_bin, lines, 1, CV_PI/2, 100);
	if(lines.size() <1){
		printf("invalid img\n");
		return  -1;
	}
	for( i = 0; i < lines.size(); i++ )
	{
		float rho = lines[i][0], theta = lines[i][1];
		
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + width*(-b));
		pt1.y = cvRound(y0 + width*(a));
		pt2.x = cvRound(x0 - width*(-b));
		pt2.y = cvRound(y0 - width*(a));
		line( m_line, pt1, pt2, Scalar(0,0,255), 1, CV_AA);
	}
	#else
        vector<Vec4i> lines;
        HoughLinesP( m_bin, lines, 1, CV_PI/2, 150, 150 );
        for( size_t i = 0; i < lines.size(); i++ )
        {
            line( m_line, Point(lines[i][0], lines[i][1]),
                Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 0.3, CV_AA );
        }
    #endif
	PointClustering(lines,g_lineY);
	imwrite("4Line.jpg", m_line);
    
    //s2 get contour
	Canny( m_bin, g_cannyMat_output, 20, 255);
    findContours(m_bin, g_Contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    Mat markers = Mat::zeros(m_bin.size(), CV_32SC1);
    
    for ( i = 0; i < g_Contours.size(); i++)
        drawContours(markers, g_Contours, static_cast<int>(i), Scalar::all(static_cast<int>(i)+1), -1);
    
    circle(markers, Point(5, 5), 3, CV_RGB(255, 255, 255), -1);
    imwrite("5contours.jpg", markers);
    
    //s3 get center monments
    vector<Moments> mu(g_Contours.size() );
    for(unsigned int i = 0; i < g_Contours.size(); i++ )
    { 
		mu[i] = moments( g_Contours[i], false ); 
	}
    
    //  calc mc
    vector<Point2f> mc( g_Contours.size() );
    for( unsigned int i = 0; i < g_Contours.size(); i++ )
    {
        mc[i] = Point2i( static_cast<float>(mu[i].m10/mu[i].m00), static_cast<float>(mu[i].m01/mu[i].m00 )) ;
    }
    
    
    // drawing contour and calc args of mc
    Mat drawing = Mat::zeros( g_cannyMat_output.size(), CV_8UC3 );
    Scalar color = CV_RGB(0,255,0);
    //strAreaTag  area;
	StrMonmentTag mcTag;
	int d,s;
	g_MonmentTag.clear();
    for( unsigned int i = 0; i< g_Contours.size(); i++ )
    {	
        d =(long int) arcLength( g_Contours[i], true );
        s =(int) contourArea(g_Contours[i]);		
        if(d < FL_MIN_LENGTH || s < FL_MIN_AREA){
            continue;
        }	
		mcTag.d = d;
        mcTag.s = s;
		mcTag.mc = mc.at(i);
		mcTag.contourIndex = i;		
		printf("[%d],mc.x=%f,mc.y=%f, s:%d ,d:%d \n", i,mc[i].x,mc[i].y, (int)mcTag.s, (int)mcTag.d);
		drawContours( drawing, g_Contours, i, color, 2, 8, g_vHierarchy, 0, Point() );     
        circle( drawing, mc[i], 3, Scalar(0,0,255), -1, 0.1f, 0 );
        putText(drawing,int2str(i,mcTag.s,mcTag.d),mc[i],CV_FONT_HERSHEY_DUPLEX,0.3f,Scalar(255,255,255));
		putText(drawing,int2str(mc[i].x, mc[i].y),Point2i(mc[i].x-40,mc[i].y+10),CV_FONT_HERSHEY_DUPLEX,  0.3f,Scalar(0,0,255));
		g_MonmentTag.push_back(mcTag);
	}
	imwrite("6mc.jpg", drawing);
	if(g_MonmentTag.size() >2 ){
	    std::sort(g_MonmentTag.begin(),g_MonmentTag.end(),SortCmp);
	}

	//s4 candidate,match to lines
	strAreaTag area;
	for( size_t i = 0; i < g_lineY.size(); i++){
	   area.lineIndex = i;
       area.mcIndex = Match2Moment(g_MonmentTag, g_lineY.at(i));
	   //area.mc = g_MonmentTag.at(area.mcIndex);
	   area.captruePoint = Point2i(IMG_WIDTH/2, g_lineY.at(i));	   
	   area.boxUp = CalcWidth(m_bin,area.captruePoint,10,CALC_WIDTH_UP);
	}

	return 1;
	    //iRet = GetCandicatArea(&area, drawing.cols,drawing.rows);
        if(iRet > 0){
            color = Scalar(0, 0xff, 0xff);//yellow
        }else{
            color = Scalar(0, 255,0);
        }
   //     drawContours( drawing, g_Contours, i, color, 2, 8, g_vHierarchy, 0, Point() );     
   //     circle( drawing, mc[i], 3, Scalar(0,255,0), -1, 0.1f, 0 );
//        putText(drawing,int2str(i,g_AllArea.at(i).s,g_AllArea.at(i).d),mc[i],CV_FONT_HERSHEY_DUPLEX,0.5f,Scalar(255,255,255));
        //putText(drawing,int2str(area.moments.x, area.moments.y),Point2i(mc[i].x,mc[i].y-20),CV_FONT_HERSHEY_DUPLEX,  0.5f,Scalar(100,100,0));  
	//std::sort(g_CandidateArea.begin(),g_CandidateArea.end(),SortCmp);
//	CalcDist(&g_CandidateArea,drawing.cols,drawing.rows);
	const Point * pts[1] = {g_CandidateArea.at(0).AverPoint};
	cv::fillPoly(drawing,pts, &g_CandidateArea.at(0).AverCount,1,cv::Scalar(0xff,0xff,0)); 
	char dist[40];
    itoa(g_CandidateArea.at(0).captruePoint.y,  dist,   10);
	strcat(dist," mm");
	putText(drawing,dist,Point2f(g_CandidateArea.at(0).captruePoint.x,g_CandidateArea.at(0).captruePoint.y-50),
		                 CV_FONT_HERSHEY_DUPLEX,0.7f,Scalar(0,0,255));
	cv::arrowedLine(drawing,Point(g_CandidateArea.at(0).captruePoint.x,0),g_CandidateArea.at(0).captruePoint,Scalar(0,0,255),2);

    cv::putText(drawing,"Sum:",Point2f(0,50),CV_FONT_HERSHEY_DUPLEX,1.0f,color);
    cv::putText(drawing,int2str(g_CandidateArea.size()),Point2f(80,50),CV_FONT_HERSHEY_SIMPLEX,0.8f,Scalar(255,255,255));

	cv::rectangle(drawing,Point(width-200,0),Point(width-170,20),Scalar(0, 0xff, 0xff),2);
	cv::putText(drawing,"Candidate area",Point2f(width-160,20),CV_FONT_HERSHEY_DUPLEX,0.6f,Scalar(0, 0xff, 0xff));

	cv::rectangle(drawing,Point(width-200,25),Point(width-170,40),Scalar(0, 0, 0xff),2);
	cv::putText(drawing,"Dest area",Point2f(width-160,40),CV_FONT_HERSHEY_DUPLEX,0.6f,Scalar(0, 0, 0xff));
    imwrite( "6draw.jpg", drawing );
	waitKey();
    return 0;
}
