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
#include <iostream>
#include <deque>

#define  FL_MIN_LENGTH   100
#define  FL_MIN_AREA     70
#define  DELTA           (6)  // contour error   1/6 *w
#define  DELTA_POINT     (10)  // point error  1/10 *w
#define  SUM_CALC_POINT   (100)
//#define  MIN_X           200
#define  MIN_Y           300
#define  MIN_CANDIDAT_DIAMETER    200
#define  MIN_CANDIDAT_AREA        1000

vector<vector<Point> > g_Contours;
vector<strAreaTag> g_AllArea,g_CandidateArea;
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
int SortCmp(strAreaTag a1, strAreaTag a2)
{
	return a1.moments.y < a2.moments.y;
}
int CalcDist( vector<strAreaTag> * pData, int w, int h)
{
	int Cx = w>>1;
	int Cy = h>>1;
	int delta = 0;
	unsigned long int sum= 0;
	int i = 0,j = 0,pos =0,tmp=w;


	//s1 find 最外围中心点，以mc(Xc,Yc).Xc为定点，取Y最小值
	for( vector<Point>::iterator it = g_Contours.at(pData->at(0).index).begin(); 
		 it != g_Contours.at(pData->at(0).index).end();it++){
		if( abs( (*it).x - pData->at(0).moments.x) <  pData->at(0).d/8 ){	
			
			if((*it).y < tmp){
				pos = i;
			    tmp = (*it).y;
				printf("find mc[%d]:%d,%d\n",pos,(*it).x, (*it).y );
			}
		}
		i++;
	}	  

	//s2 计算最外围点横向<100 邻居点x的均值Xe，做深度采样点(Xe,Yc)
	pData->at(0).AverCenterIndex = pos;
	pData->at(0).captruePoint = g_Contours.at(pData->at(0).index).at(pos);
	if(g_Contours.at(pData->at(0).index).size() > SUM_CALC_POINT *2 ){
		delta = SUM_CALC_POINT;
	}else{
		delta = g_Contours.at(pData->at(0).index).size() >> 1;
	}

	for( i=pos-delta,j=0; i< pos+delta; i++){
		if(i>=0 &&  i< g_Contours.at(pData->at(0).index).size() ) // [0,size] ?
			if(g_Contours.at(pData->at(0).index).at(i).x > pData->at(0).x1  &&
			   g_Contours.at(pData->at(0).index).at(i).x < pData->at(0).x2  &&
			   g_Contours.at(pData->at(0).index).at(i).y < pData->at(0).y2  &&
			   g_Contours.at(pData->at(0).index).at(i).y > pData->at(0).y1  ){
				sum += g_Contours.at(pData->at(0).index).at(i).x;
				pData->at(0).AverPoint[j].x = g_Contours.at(pData->at(0).index).at(i).x;
				pData->at(0).AverPoint[j].y = g_Contours.at(pData->at(0).index).at(i).y;
				j++;
			}
	}
	if(j)  {
		pData->at(0).AverCount = j;
		pData->at(0).AverValue = sum /j;
	}
	return 0;
}
//寻找二值图像上的第一个点

//maskImg二值图像,标记

//outPoint输出查找的点

bool findFirstPoint(Mat &maskImg, Point &outPoint)

{

	bool success = false;

	for (int i = 0; i < maskImg.rows; i++)

	{

		unsigned char *pData = (unsigned char*)(maskImg.data+i*maskImg.step);

		for (int j = 0; j < maskImg.cols; j++)

		{

			if (pData[j] == 255)//找到第一个白色的点

			{

				success = true;

				outPoint.x = j;

				outPoint.y = i;

				pData[j] = 0;//将此点像素设为0

				break;

			}

		}

		if (success)

			break;

	}

	return success;

}
//八邻域中寻找曲线上某个点的下一个点
//neiPoint八邻域点
//currPt当前点
bool findNextPoint(vector<Point> &neiPt, Mat &image, Point currPt, int currFlag, Point& outPt, int &outFlag)

{
	int i = currFlag;//0，6

	int count = 1;

	bool success = false;

	while (count <= 7)
	{
		Point tpPt = currPt + neiPt[i];//邻域像素点

		if (tpPt.x > 0 && tpPt.y > 0 && tpPt.x < image.cols && tpPt.y < image.rows)//在图像内部

		{

			if( ((unsigned char*)(image.data+tpPt.y*image.step))[tpPt.x]==255 )

			{

				outPt = tpPt;

				outFlag = i;

				success = true;

				((unsigned char*)(image.data+tpPt.y*image.step))[tpPt.x]=0;

				break;//跳出循环

			}

		}

		if (count % 2)//奇数

		{

			i += count;

			if (i > 7)

			{

				i -= 8;

			}

		}

		else

		{

			i += -count;

			if (i < 0)

			{

				i += 8;

			}

		}
		count++;
	}
	return success;
}
//寻找曲线 ：
void findLines(Mat &binaryImg, vector<deque<Point>> &outLines)

{

	//八邻域

	vector<Point> neighborPtVec;

	neighborPtVec.push_back(Point(-1,-1));

	neighborPtVec.push_back(Point(0,-1));

	neighborPtVec.push_back(Point(1,-1));

	neighborPtVec.push_back(Point(1,0));



	neighborPtVec.push_back(Point(1,1));

	neighborPtVec.push_back(Point(0,1));

	neighborPtVec.push_back(Point(-1,1));

	neighborPtVec.push_back(Point(-1,0));



	Point firstPt;

	while (findFirstPoint(binaryImg, firstPt))

	{

		deque<Point> line;//点的队列

		line.push_back(firstPt);//存储第一个点



		//由于第一个点不一定是线段的起始位置，

		//因此两个方向都要查找

		Point currPt = firstPt;//当前点

		int currFlag = 0;//标志

		Point nextPt;//下一个点

		int nextFlag;//下一点的标志

		while (findNextPoint(neighborPtVec, binaryImg, currPt, currFlag, nextPt, nextFlag))//一端

		{

			line.push_back(nextPt);//压入队列

			currPt = nextPt;

			currFlag = nextFlag;

		}

		//找另一端

		currPt = firstPt;

		currFlag = 0;

		while (findNextPoint(neighborPtVec, binaryImg, currPt, currFlag, nextPt, nextFlag))

		{

			line.push_front(nextPt);

			currPt = nextPt;

			currFlag = nextFlag;//邻域与中心像素的位置

		}

		if (line.size() > 10)

		{

			outLines.push_back(line);

		}

	}

}
//用deque<Point> 描述曲线

//随机取色画曲线

Scalar random_color(RNG& _rng)

{

	int icolor = (unsigned)_rng;

	return Scalar(icolor & 0xFF, (icolor >> 8) & 0xFF, (icolor >> 16) & 0xFF);

}
int main()
{
	Mat gray,Dest;
	Mat bw = Mat::zeros(gray.size(),CV_8U);
	Mat img = imread("D:/prj/CvCounter/z1.bmp", 1);
	//cvtColor(img,bw,COLOR_BGR2GRAY);
	GaussianBlur(img, bw, Size(15, 15), 0.5);
	printf("bw, depth:%d,channel:%d\n",bw.depth(),bw.channels());

	//s1 canny	
	//equalizeHist(img,bw);
	//threshold(gray, bw, 150, 200, CV_THRESH_OTSU);
	imwrite("2bin.bmp", bw);

	Mat g_cannyMat_output = Mat::zeros(bw.size(),CV_8U);
	Canny( bw, g_cannyMat_output,150, 254,3,true);
	//Sobel(bw,g_cannyMat_output,CV_8UC1,1,1,1);
	//Scharr(bw,g_cannyMat_output,CV_8UC1,0,1,1,3);
	//Laplacian(bw,g_cannyMat_output,CV_8U,1,1,0,4);
	imwrite("2canny.bmp",g_cannyMat_output);
	//filter2D();	//get lines 
	vector<deque<Point>> lines;
	findLines(g_cannyMat_output, lines);
	cout << lines.size() << endl;

	//draw lines
	Mat draw_img;
	cvtColor(g_cannyMat_output,draw_img,CV_GRAY2BGR);
	RNG rng(123);
	Scalar color;
	for (int i = 0; i < lines.size(); i++)
	{
    	color = random_color(rng);
		for (int j = 0; j < lines[i].size(); j++)
		{		
			draw_img.at<Vec3b>(lines[i][j]) = Vec3b(color[0], color[1], color[2]);
		}
	}
	imshow("draw_img", draw_img);
	imwrite("draw_img.jpg", draw_img);
	waitKey(0);
	return 0;
}


int main2()
{
    int iRet = 0;
    int mc_valid[300] = {1};
    RNG g_rng(12345);
    Mat g_cannyMat_output;
    vector<Vec4i> g_vHierarchy;
    Mat img = imread("D:/prj/CvCounter/z1.jpg", 1);
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
        //g_AllArea.at(i)->moments = &mc;
    }
    
    
    // s4 drawing contour and calc args of mc
    Mat drawing = Mat::zeros( g_cannyMat_output.size(), CV_8UC3 );
    Scalar color = CV_RGB(0,255,0);
    int sum = (int)g_Contours.size();
    //int j =0;
    strAreaTag  area;

    for( unsigned int i = 0; i< g_Contours.size(); i++ )
    {
		memset(&area,0,sizeof(strAreaTag));
        area.d =(long int) arcLength( g_Contours[i], true );
        area.s =(int) contourArea(g_Contours[i]);
		area.moments = mc.at(i);
		area.index = i;		
		
		g_AllArea.push_back(area);
        if(area.d < FL_MIN_LENGTH || area.s < FL_MIN_AREA){
            continue;
        }		
		printf("[%d], s:%d ,d:%d \n", i, (int)area.s, (int)area.d);
		AreaGetMax(&area, drawing.cols, drawing.rows);
        iRet = GetCandicatArea(&area, drawing.cols,drawing.rows);
        if(iRet > 0){
            color = Scalar(0, 0xff, 0xff);//yellow
        }else{
            color = Scalar(0, 255,0);
        }
        drawContours( drawing, g_Contours, i, color, 2, 8, g_vHierarchy, 0, Point() );     
        circle( drawing, mc[i], 3, Scalar(0,255,0), -1, 0.1f, 0 );
        putText(drawing,int2str(i,g_AllArea.at(i).s,g_AllArea.at(i).d),mc[i],CV_FONT_HERSHEY_DUPLEX,0.5f,Scalar(255,255,255));
        putText(drawing,int2str(area.moments.x, area.moments.y),Point2i(mc[i].x,mc[i].y-20),CV_FONT_HERSHEY_DUPLEX,  0.5f,Scalar(100,100,0));
        
    }
	std::sort(g_CandidateArea.begin(),g_CandidateArea.end(),SortCmp);
	CalcDist(&g_CandidateArea,drawing.cols,drawing.rows);
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
    imwrite( "4draw.jpg", drawing );
	waitKey();
    return 0;
}
