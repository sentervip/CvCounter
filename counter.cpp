#include "counter.hpp"
#include "fstream"
#include <iostream>
#include <string>
#include <strstream>
#include "math.h"
#include <stdlib.h>
#include<algorithm> 
#include <vector>

#define  IMG_WIDTH      640
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
#define  MIN_DISTANCE_CLUSTER     25
#define  MIN_IMG_Y                20 //MIN_DISTANCE_CLUSTER
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
    for(vector<cv::Point2f>::iterator iter = mc.begin(); iter != mc.end(); ){
        if(i>0) //s1: repeate
            if( abs(iter->x - pt_tmp.x) < 2 || abs(iter->y - pt_tmp.y) < 2 ){
                mc.erase(iter);
                cout<<"erase i: "<<i<<std::endl;
                putText(drawing,int2str(i),pt_tmp,CV_FONT_HERSHEY_DUPLEX,0.8f,CV_RGB(255,0,0));
                valid[i] = 0;
                j++;
			}else{
				iter++;
			}
        i++;
        pt_tmp = *iter;
        
    }
    
    return (i - j+1);
}
void AreaGetMax(strAreaTag * area, int w, int h)
{    
}

int SortCmp(StrMonmentTag a1, StrMonmentTag a2)
{
	return a1.s > a2.s;
}
int SortCmpUp(int a1, int a2)
{
	return a1 < a2;
}
int SortArea(strAreaTag a1, strAreaTag a2)
{
	return g_MonmentTag.at(a1.mcIndex).s > g_MonmentTag.at(a2.mcIndex).s;
}
int SortDoc(strAreaTag a1, strAreaTag a2)
{
	return a1.doc > a2.doc;
}
int SortUp(strAreaTag a1, strAreaTag a2)
{
	return a1.boxUp > a2.boxUp;
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
	if(_pts.size() <=1){
		printf("%s, size：%d\n",__FUNCTION__, _pts.size());
		return 0;
	}
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
					break;
				}
			}
		}


	}
	return 0;
}
int LineMatch2Moment(vector<StrMonmentTag> MomentTag,  int lineY)
{
    vector<StrMonmentTag>::iterator it;
	int index = 0;
	int i=0;
	int Min = 500;
	int iAbs;

	for(it=MomentTag.begin();it != MomentTag.end(); it++){
		iAbs = abs(it->mc.y - lineY);
		if( iAbs < Min 
			&& it->s > FL_MIN_AREA  
			&& (it->d > FL_MIN_LENGTH) ){
			Min = iAbs;
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
	int w = 0;

		
		if(UpOrDown == CALC_WIDTH_UP){
			while(--pt.y > MIN_IMG_Y){
					sum = 0;
					for(int i=-len/2;i<len/2; i++){
						sum += bin.at<uchar>(pt.y,IMG_WIDTH/2+i);
						//sum += bin.at<uchar>(1,IMG_WIDTH/2+i);
					}
				aver = sum/len;
				if( aver < 0.3 *0xff){ //有效黑色区域计算			
					w++;
				}else if(w == 0){ //在本白线内部
					printf("waring, %s,innerLine %d",__FUNCTION__, pt.y);
					continue;
				}else if( aver > 0.7 *0xff){
					break;
				}
			}
	    }else{
			while(++pt.y < IMG_HEIGH){
				sum = 0;
				for(int i=-len/2;i<len/2; i++){
					sum += bin.at<uchar>(pt.y,IMG_WIDTH/2+i);
					//sum += bin.at<uchar>(1,IMG_WIDTH/2+i);
				}
				aver = sum/len;
				if( aver < 0.3 *0xff){ //有效黑色区域计算			
					w++;
				}else if(w == 0){ //在本白线内部
					printf("waring, %s,innerLine %d",__FUNCTION__, pt.y);
					continue;
				}else if( aver > 0.7 *0xff){
					break;
				}
			}
	}
	return w;
}
int main()
{
	char dist[40];
    int i,j,iRet = 0;
	float depth;
	Point pt1, pt2;
    int mc_valid[300] = {1};
    RNG g_rng(12345);
    Mat g_cannyMat_output;
    vector<Vec4i> g_vHierarchy;
	vector<int> g_lineY;
	Mat DestRGB;
    Mat m_origin = imread("D:/prj/z1/src/img/0916/1.png", 1.0);
	Mat m_gray;
    
    //s1 preprocess     
    int w = m_origin.cols;
    int h = m_origin.rows;
	if(m_origin.data == NULL){
		printf("can not open 1.png\n");
		waitKey(0);
	}
    Mat _m_gray = Mat::zeros(m_origin.size(),CV_8UC1);
    for ( i = 0; i < h; i++)
    {
        for ( j = 0; j < w; j++)
        {
            int pix = m_origin.at<Vec3b>(i, j)[0] + m_origin.at<Vec3b>(i, j)[2] - m_origin.at<Vec3b>(i, j)[1];
            if (pix > 255)
                pix = 255;
            if (pix < 0)
                pix = 0;
            _m_gray.at<uchar>(i, j) = (uchar)pix;
        }
    }
	imwrite("0gray.jpg", _m_gray);
	GaussianBlur(_m_gray, m_gray, Size(15, 15), 1.0);  

    //morphology	
	int g_nElementShape = MORPH_RECT;
	Mat element = getStructuringElement(g_nElementShape, Size(5, 5) );
	morphologyEx(m_gray, m_gray, CV_MOP_ERODE, element);  imwrite("1erode.jpg",m_gray);
	element = getStructuringElement(g_nElementShape, Size(7, 7) );
	morphologyEx(m_gray, m_gray, CV_MOP_DILATE, element);
    imwrite("2dilate.jpg",m_gray);
    Mat m_bin;
    
    //Bin
    threshold(m_gray, m_bin, 20, 255, CV_THRESH_OTSU);
    imwrite("3bin.jpg", m_bin);
	
	//lines
	Mat m_line(m_bin.size(),CV_8UC1);

	#if 1
	vector<Vec2f> lines; 
	HoughLines(m_bin, lines, 1, CV_PI/2, 100);
	for(vector<Vec2f>::iterator it = lines.begin(); it != lines.end(); )
	{
		//float rho = lines[i][0], theta = lines[i][1];
//		float rho = it->val[0];
//		float theta = it->val[1];;
//		double a = cos(theta), b = sin(theta);
//		double x0 = a*rho, y0 = b*rho;
//		pt1.x = cvRound(x0 + w*(-b));
//		pt1.y = cvRound(y0 + w*(a));
//		pt2.x = cvRound(x0 - w*(-b));
//		pt2.y = cvRound(y0 - w*(a));
		if(it->val[0] < MIN_IMG_Y){
			it = lines.erase(it);
		}else{
			//line( m_line, pt1, pt2, Scalar(0,0,255), 1, CV_AA);
			it++; 
		}
	}
	if(lines.size() <1){
		printf("invalid img\n");
		return  EINVALID_ARG;
	}else if(lines.size() == 1){
		//measure and show		
		cv::cvtColor(m_gray,DestRGB,COLOR_GRAY2RGB);
		memset(dist,0,sizeof(dist));
		depth = lines.at(0).val[0]; // 1540/m_osc*sampeRate/1000000*iRet;
		sprintf(dist," %0.1f mm", depth);
		putText(DestRGB,dist,Point2i(IMG_WIDTH/2+5,lines.at(0).val[0]/2),CV_FONT_HERSHEY_DUPLEX,0.4f,Scalar(0,0,255));
		cv::arrowedLine(DestRGB,Point(IMG_WIDTH/2,0),Point2i(w/2,lines.at(0).val[0]),Scalar(0,0,255),1);
		imwrite( "Dest.jpg", DestRGB );
		//waitKey();
		return depth;
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
		for(std::vector<int>::iterator it=g_lineY.begin(); it != g_lineY.end();){
			line( m_line, Point(0,*it), Point(w, *it), Scalar(0,0,255), 1, CV_AA);
			it++;
		}
		imwrite("4Line.jpg", m_line);
    
    //s2 get contour
	Mat bMat = m_bin.clone();
	Canny( bMat, g_cannyMat_output, 20, 255);
    findContours(bMat, g_Contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    Mat markers = Mat::zeros(bMat.size(), CV_32SC1);
    
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
	}else if(g_MonmentTag.size() <=1){
	    printf("please touch up");
	    return ECLOSED;
	}else;
    printf("tour over");

	//s4 candidate,match to lines, bug: 靠近前的多线数量 > 闭环区域数 导致后面漏线匹配情况
	strAreaTag area;
	i = 0;
	std::sort(g_lineY.begin(),g_lineY.end(), SortCmpUp);
	for( vector<int>::iterator it = g_lineY.begin(); it !=g_lineY.end(); i++){
	   if(g_lineY.at(i) < MIN_IMG_Y){
		   it =g_lineY.erase(it);
		   continue;
	   }
	   area.lineIndex = i;
       area.mcIndex = LineMatch2Moment(g_MonmentTag, g_lineY.at(i));
	   //area.mc = g_MonmentTag.at(area.mcIndex);
	   area.s = g_MonmentTag.at(area.mcIndex).s;
	   area.captruePoint = Point2i(IMG_WIDTH/2, g_lineY.at(i));	   
	   area.boxUp = CalcWidth(m_bin,area.captruePoint,10,CALC_WIDTH_UP);
	   //area.boxDown = CalcWidth(m_bin,area.captruePoint,10,CALC_WIDTH_DOWN);
	   g_CandidateArea.push_back(area);

	   // g_MonmentTag.size < g_lineY.size ? break
	   if( i>= g_MonmentTag.size()){
		   break;
	   }
	    it++;
	}
	vector<strAreaTag> _CandidateArea(g_CandidateArea);      
	std::sort(g_CandidateArea.begin(),g_CandidateArea.end(),SortArea);
	std::sort(_CandidateArea.begin(),_CandidateArea.end(),SortUp);
	for( size_t i = 0; i < g_lineY.size(); i++){
		g_CandidateArea.at(i).wws = g_CandidateArea.at(i).s * 100/g_CandidateArea.at(0).s ; 
		j = g_lineY.size();                                          
		while(--j >= 0){
			if(g_CandidateArea.at(i).lineIndex == _CandidateArea.at(j).lineIndex){
				g_CandidateArea.at(i).bws = _CandidateArea.at(j).boxUp*100/_CandidateArea.at(0).boxUp;
				break;
			}
		}
		g_CandidateArea.at(i).doc = g_CandidateArea.at(i).bws + g_CandidateArea.at(i).wws;
	}
	int DestLine = 0;
	std::sort(g_CandidateArea.begin(),g_CandidateArea.end(),SortDoc);
	if(g_CandidateArea.at(0).doc == g_CandidateArea.at(1).doc){
		if(g_CandidateArea.at(0).wws > g_CandidateArea.at(1).wws){
			DestLine = 0;
		}else{
			DestLine = 1;
		}
	}

SHOW_DEST:
    //measure and show
   // Mat DestRGB;
    cv::cvtColor(_m_gray,DestRGB,COLOR_GRAY2RGB);
    //char dist[40];
    memset(dist,0,sizeof(dist));
    //depth = 1540/32/1000000 *3 * g_CandidateArea.at(DestLine).captruePoint.y*1000; // 1540/m_osc*sampeRate/1000000*iRet;
    //iRet = (int) round(depth);
	depth = g_CandidateArea.at(DestLine).captruePoint.y;
    sprintf(dist," %0.0f mm", depth);
    //printf("dist:%s", dist);
	putText(DestRGB,dist,Point2i(IMG_WIDTH/2+5,g_CandidateArea.at(DestLine).captruePoint.y/2),CV_FONT_HERSHEY_DUPLEX,0.4f,Scalar(0,0,255));
	cv::arrowedLine(DestRGB,Point(IMG_WIDTH/2,0),g_CandidateArea.at(DestLine).captruePoint,Scalar(0,0,255),1);
	imwrite( "Dest.jpg", DestRGB );
	waitKey();
    return 0;
}
