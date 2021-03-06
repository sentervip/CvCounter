#include "counter.hpp"
#include "fstream"
#include <iostream>
#include <string>
#include <strstream>
#include "math.h"
#include <stdlib.h>
#include <algorithm> 
#include <vector>
#include<opencv2\opencv.hpp>
using namespace cv;
using namespace std;
#define  IMG_WIDTH      200
#define  IMG_HEIGH      480

#define  FL_MIN_LENGTH   (IMG_WIDTH*0.6)
#define  FL_MIN_AREA     400
#define  DELTA           (6)  // contour error   1/6 *w
#define  DELTA_POINT     (10)  // point error  1/10 *w
#define  SUM_CALC_POINT   (100)

#define  MIN_CANDIDAT_DIAMETER    200
#define  MIN_CANDIDAT_AREA        700
#define  MIN_DISTANCE_CLUSTER     20
#define  PART_WAIST_MIN_DEPTH     4  // mm
#define  PART_FACE_MIN_DEPTH      3 // mm
#define  PART_ARM_MIN_DEPTH       3  // mm
#define  PART_CHEST_MIN_DEPTH     4
#define  PART_THIGH_MIN_DEPTH     2
#define  PART_BELLY_MIN_DEPTH     6
#define  MIN_IMG_Y                20 //MIN_DISTANCE_CLUSTER
#define  CALC_WIDTH_UP            1
#define  CALC_WIDTH_DOWN          2
#define  K3_GAUSSIAN_SIGAMA       70
#define  K3_GAUSSIAN_C            70
#define  K3_DEPTH_LOSS_COEFI      100 // depth loss coefficient of Gaussian curve

vector<StrMonmentTag> g_MonmentTag;  //momentes struct
vector<vector<Point> > g_Contours;
vector<strAreaTag> g_AllArea,g_CandidateArea; //all area and area of lines
vector<strAreaTag> * pAreaTag = &g_AllArea;
int part = PART_WAIST;

#define INPUT_TITLE "input image"
#define OUTPUT_TITLE "hist calc"

using namespace std;
using namespace cv;


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
static int GetMinDepth(int part)
{
    switch(part){
        case PART_FACE: return PART_FACE_MIN_DEPTH;
        case PART_ARM: return PART_ARM_MIN_DEPTH;
        case PART_WAIST: return PART_WAIST_MIN_DEPTH;
        case PART_THIGH: return PART_THIGH_MIN_DEPTH;
		case PART_CHEST: return PART_CHEST_MIN_DEPTH;
		case PART_BELLY: return PART_BELLY_MIN_DEPTH;
        default: printf("%s,invalid part：%d",__FUNCTION__,part);
                 return PART_FACE_MIN_DEPTH;
    }
}
int SortPoint(vector<Point2f> mc, int * valid, Mat &drawing)
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
	if(_pts.size() <=0){
		printf("%s, size：%d\n",__FUNCTION__, _pts.size());
		return -1;
	}else if(_pts.size() ==1){
		pts.push_back(cvRound(_pts[0][0]) );
		out.push_back(pts.at(0));
	    return 1;
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
int GetEdge(cv::Mat &bin, Point2i SrcPt)
{
	int min =0;
	int w1 =0;
	int w11 =0;
	int w2 = 0;
    Point2i pt = Point2i(SrcPt.x,SrcPt.y);

	if( bin.at<uchar>(pt.y,pt.x) > 0){  //1 pointer in the line
		while(--pt.y > MIN_IMG_Y){
			w1++;
			if ( bin.at<uchar>(pt.y,pt.x) == 0)  break;
		}
		return SrcPt.y-w1;
	}else{                                   //2 point out of line
		while(--pt.y > MIN_IMG_Y){ //get width of upper
			w1++;
			if( bin.at<uchar>(pt.y,pt.x) > 0)  {  //遇到白色-》黑色--》结束
				w11 = w1;
				while(--pt.y > MIN_IMG_Y){
					w11++;
					if( bin.at<uchar>(pt.y,pt.x) == 0){
						break;
					}						
				}
				break;
			}
		}
		while(++pt.y < IMG_HEIGH){ //get width lower
			w2++;
            if ( bin.at<uchar>(pt.y,pt.x) > 0)  break;
		}
		if( w2 < w1 ){
		    return SrcPt.y+w2;
		}else{
			return SrcPt.y-w11;
		}
	}
	return SrcPt.y;

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
				}else if(w == 0){ //在本白线内部，跳过
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
/**
gaussian function:
y = e^((- (x-C)^2) / (2* (siga^2)) );
*/
float getGaussianData(int x)
{
    float  cofe1,cofe2,out,e=2.71828;

	cofe1 = std::pow((float)x-K3_GAUSSIAN_SIGAMA,(float)2);
	cofe2 = std::pow((float)K3_GAUSSIAN_C,(float)2) * 2;
	out = std::pow((float)e,(float)(-cofe1/cofe2)) ;
	return out;
}
int checkImage(Mat SrcBin)
{
    Mat Src,dest;
	Mat mean,stddev;
	int iRet = 0;
	int i,j,sum = 0;
	if(SrcBin.size <= 0){
		return EINVALID_ARG;
	}

	//is empty pictrue ?
	Mat ROI = SrcBin(cv::Rect( SrcBin.cols/2-10,MIN_IMG_Y, 10, SrcBin.rows-MIN_IMG_Y));
	for(i=0;i<ROI.rows; i++){
		for(j=0;j<ROI.cols;j++)
			sum += ROI.at<uchar>(i,j);
	}
	if(sum < 5000){
		iRet = ENULL_WAVE;
	}
	return iRet;
}
int main2()
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
    Mat m_origin = imread("D:/prj/z1/src/img/200109x480x200/limit4.jpg", IMREAD_COLOR);
	//Mat m_origin = imread("D:/prj/z1/src/CvCounter/win32/1.jpg", IMREAD_COLOR);

	Mat m_gray;  
    //s1 preprocess     
    int w = m_origin.cols;
    int h = m_origin.rows;
	if(m_origin.data == NULL){
		printf("can not open 1.png\n");
		//waitKey(0);
		std::cin >> depth;
		return EINVALID_ARG;
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
	
	_m_gray = _m_gray.mul(1.4);
	if(part == PART_BELLY){		
		GaussianBlur(_m_gray, _m_gray, Size(3, 3) , 1.0); 
	}else{
		Mat m_grayEnhance;
		GaussianBlur(_m_gray, m_grayEnhance, Size(3, 3), 1.0);  
		cv::equalizeHist(_m_gray, m_grayEnhance);
		imwrite("1.1m_grayEnhance.jpg",m_grayEnhance); 
		//GaussianBlur(m_grayEnhance, m_grayEnhance, Size(5, 5) , 1.0);    imwrite("1.2Blur.jpg",m_grayEnhance); 

		//morphology	
		int g_nElementShape = MORPH_RECT;
		Mat element = getStructuringElement(g_nElementShape, Size(3, 3) );
		morphologyEx(m_grayEnhance, m_gray, CV_MOP_ERODE, element);  
		imwrite("1erode.jpg",m_gray);
		element = getStructuringElement(g_nElementShape, Size(3, 3) );
		morphologyEx(m_gray, m_gray, CV_MOP_DILATE, element);
		imwrite("2dilate.jpg",m_gray);
	}
    
    
    //Bin
	Mat m_bin;	
    threshold(m_gray, m_bin, 20, 255, CV_THRESH_OTSU);
    imwrite("3bin.jpg", m_bin);
	
	//Get lines
	//Mat m_line(m_bin.size(),CV_8UC1);
	Mat m_line(h,w,0);

	#if 1
	vector<Vec2f> lines; 
	iRet = checkImage(m_bin);
	if(iRet < 0){
		return iRet;
	}
	HoughLines(m_bin, lines, 1, CV_PI/2, IMG_WIDTH-5);
	for(vector<Vec2f>::iterator it = lines.begin(); it != lines.end(); )
	{
		if(it->val[0] < MIN_IMG_Y){
			it = lines.erase(it);
		}else{
			//line( m_line, pt1, pt2, Scalar(0,0,255), 1, CV_AA);
			it++; 
		}
	}
	
	//cluster and sort lines
	iRet = PointClustering(lines,g_lineY);
	if(iRet < 0){
		printf("cluster failed,exit");
		//waitKey(0);
		std::cin >> depth;
		return ELOW_WAVE;
	}
	std::sort(g_lineY.begin(),g_lineY.end(), SortCmpUp);
	for(std::vector<int>::iterator it=g_lineY.begin(); it != g_lineY.end();){
		cv::line( m_line, Point(0,*it), Point(w, *it), Scalar(255,255,255), 1);
		it++;
	}
	imwrite("4Line.jpg", m_line);

	if(g_lineY.size() <1){  //没有找到线条
		printf("invalid img\n");
		std::cin >> depth;
		return  ENULL_WAVE;
	}else if( g_lineY.size() == 1 || part == PART_FACE || part == PART_ARM || part==PART_THIGH){  //只有一条线
		//measure and show		
		cv::cvtColor(_m_gray,DestRGB,COLOR_GRAY2RGB);
		memset(dist,0,sizeof(dist));
		depth = g_lineY.at(0); // 1540/m_osc*sampeRate/1000000*iRet;
		depth = GetEdge(m_bin,cv::Point2i(IMG_WIDTH/2,depth));
		if(depth < GetMinDepth(part)){
			printf("depth < 6mm,again\n");
			//waitKey(0);
			std::cin >> depth;
			return  ELOW_WAVE;
		}
		sprintf(dist," %d mm", cvRound(depth));
		putText(DestRGB,dist,Point2i(IMG_WIDTH/2+5,depth/2),CV_FONT_HERSHEY_DUPLEX,0.6f,Scalar(0,0,255));
		cv::arrowedLine(DestRGB,Point(IMG_WIDTH/2,0),Point2i(w/2,depth),Scalar(0,0,255),1);
		imwrite( "7Dest.jpg", DestRGB );
		printf("calc Ok,only one");
		std::cin >> depth;
		return depth;
	}else if(g_lineY.size() > 8 && part == PART_WAIST){
        printf("g_lineY.size=%d,EOVER_PRESSED_WAVE,exit\n", g_lineY.size());
		std::cin >> depth;
        return  EOVER_PRESSED_WAVE;
	}else if(g_lineY.size() >=2 && g_lineY.at(0) > IMG_HEIGH/3){  //如果第一条线超过图像1/3深度，优先选择
		cv::cvtColor(_m_gray,DestRGB,COLOR_GRAY2RGB);
		memset(dist,0,sizeof(dist));
		depth = g_lineY.at(0); // 1540/m_osc*sampeRate/1000000*iRet;
		depth = GetEdge(m_bin,cv::Point2i(IMG_WIDTH/2,depth));
		sprintf(dist," %d mm", cvRound(depth));
		putText(DestRGB,dist,Point2i(IMG_WIDTH/2+5,depth/2),CV_FONT_HERSHEY_DUPLEX,0.6f,Scalar(0,0,255));
		cv::arrowedLine(DestRGB,Point(IMG_WIDTH/2,0),Point2i(w/2,depth),Scalar(0,0,255),1);
		imwrite( "7Dest.jpg", DestRGB );
		printf("calc Ok, limit ok");
		std::cin >> depth;
		return depth;
	}else;
	#else
        vector<Vec4i> lines;
        HoughLinesP( m_bin, lines, 1, CV_PI/2, 150, 150 );
        for( size_t i = 0; i < lines.size(); i++ )
        {
            line( m_line, Point(lines[i][0], lines[i][1]),
                Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 0.3, CV_AA );
        }
    #endif

    //s2 get contour
	Mat bMat = m_bin.clone();
	Canny( bMat, g_cannyMat_output, 20, 255);
    findContours(bMat, g_Contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    Mat markers = Mat::zeros(bMat.size(), CV_32SC1);
    
    for( i = 0; i < g_Contours.size(); i++)
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
	if(g_MonmentTag.size() >=2 ){
	    std::sort(g_MonmentTag.begin(),g_MonmentTag.end(),SortCmp);
	}else if(g_MonmentTag.size() <=1){
	    printf("please touch up");
		std::cin >> depth;
	    return ELOW_WAVE;
	}else;
   

	//s4 candidate,match to lines, bug: 靠近前的多线数量 > 闭环区域数 导致后面漏线匹配情况
	strAreaTag area;
	i = 0;
	for( vector<int>::iterator it = g_lineY.begin(); it !=g_lineY.end(); i++){
	   if(g_lineY.at(i) < MIN_IMG_Y){
		   it =g_lineY.erase(it);
		   continue;
	   }
	   if( i>= g_MonmentTag.size()){ // g_MonmentTag.size < g_lineY.size ? break
		   break;
	   }
	   area.lineIndex = i;
       area.mcIndex = LineMatch2Moment(g_MonmentTag, g_lineY.at(i));
	   //area.mc = g_MonmentTag.at(area.mcIndex);
	   area.s = g_MonmentTag.at(area.mcIndex).s;
	   area.captruePoint = Point2i(IMG_WIDTH/2, g_lineY.at(i));	   
	   area.boxUp = CalcWidth(m_bin,area.captruePoint,10,CALC_WIDTH_UP);
	   area.dls =  getGaussianData(area.captruePoint.y) * K3_DEPTH_LOSS_COEFI;
	   //area.boxDown = CalcWidth(m_bin,area.captruePoint,10,CALC_WIDTH_DOWN);
	   g_CandidateArea.push_back(area);
	   it++;
	}
	vector<strAreaTag> _CandidateArea(g_CandidateArea);      
	std::sort(g_CandidateArea.begin(),g_CandidateArea.end(),SortArea); //面积排序
	std::sort(_CandidateArea.begin(),_CandidateArea.end(),SortUp);  //黑色区域排序
	for( i = 0; i < g_CandidateArea.size(); i++){ 
		g_CandidateArea.at(i).wws = g_CandidateArea.at(i).s * 100/g_CandidateArea.at(0).s ; 
		j = g_CandidateArea.size();                                          
		while(--j >= 0){
			if(g_CandidateArea.at(i).lineIndex == _CandidateArea.at(j).lineIndex){
				g_CandidateArea.at(i).bws = _CandidateArea.at(j).boxUp*100/_CandidateArea.at(0).boxUp;
				break;
			}
		}
		g_CandidateArea.at(i).doc = g_CandidateArea.at(i).bws + g_CandidateArea.at(i).wws + g_CandidateArea.at(i).dls;
	}
	int DestLine = 0;
	if(g_CandidateArea.size() >=2 ) {
        std::sort(g_CandidateArea.begin(), g_CandidateArea.end(), SortDoc);
        if (g_CandidateArea.at(0).doc == g_CandidateArea.at(1).doc) {
            if (g_CandidateArea.at(0).wws >= g_CandidateArea.at(1).wws) { //两条线谁白色区域更宽
                 DestLine = 0;
            } else {
                DestLine = 1;
            }
        }
    }

    //measure and show
   // Mat DestRGB;
    cv::cvtColor(_m_gray,DestRGB,COLOR_GRAY2RGB);
    //char dist[40];
    memset(dist,0,sizeof(dist));
    //depth = 1540/32/1000000 *3 * g_CandidateArea.at(DestLine).captruePoint.y*1000; // 1540/m_osc*sampeRate/1000000*iRet;
    //iRet = (int) round(depth);
	depth = g_CandidateArea.at(DestLine).captruePoint.y;
	depth = GetEdge(m_bin,cv::Point2i(IMG_WIDTH/2,depth));
	g_CandidateArea.at(DestLine).captruePoint.y = depth;  // calibra it
    sprintf(dist," %d mm", cvRound(depth));
    //printf("dist:%s", dist);
	putText(DestRGB,dist,Point2i(IMG_WIDTH/2+5,g_CandidateArea.at(DestLine).captruePoint.y/2),CV_FONT_HERSHEY_DUPLEX,0.6f,Scalar(0,0,255));
	cv::arrowedLine(DestRGB,Point(IMG_WIDTH/2,0),g_CandidateArea.at(DestLine).captruePoint,Scalar(0,0,255),1);
	imwrite( "7Dest.jpg", DestRGB );
	if(depth < GetMinDepth(part)){
		printf("depth < 6 mm,please again");
		return EINVALID_ARG;
	}
	printf("over,success\n");
	//waitKey(0);
	//std::cin >> depth;
	//limit check
    switch(part){
	    case PART_WAIST:
	        if(depth > LIMIT_WAIST && g_lineY.size() > 1){
	            printf("LIMIT,WAIST,size=%d,depth=%f",g_lineY.size(),depth);
				return (float)ELIMIT;
	        }
	        break;
	    case PART_ARM :
			if(depth > LIMIT_ARM && g_lineY.size() > 1){
                printf("LIMIT,ARM,size=%d,depth=%f",g_lineY.size(),depth);
                return (float)ELIMIT;
			}
	        break;
		default: break;
	}
    return 0;
}

int main() 
{
   Mat src, dst;
   src = imread("D:/prj/z1/src/img/200109x480x200/1m_grayEnhance.jpg",IMREAD_COLOR);
   if (!src.data)
   {
		cout << "ERROR : could not load image.";
				return -1;
	}
			namedWindow(INPUT_TITLE, CV_WINDOW_AUTOSIZE);
			namedWindow(OUTPUT_TITLE, CV_WINDOW_AUTOSIZE);

			imshow(INPUT_TITLE, src);

			//分通道显示
			vector<Mat> bgr_planes;
			split(src, bgr_planes);

			//设定像素取值范围
			int histSize = 256;
			float range[] = { 0,255 };
			const float *histRanges = { range };

		//三个通道分别计算直方图
			Mat b_hist, g_hist, r_hist;
			calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRanges, true, false);
			calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRanges, true, false);
			calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRanges, true, false);
			double minValue = 0;
			double maxValue = 0;
			minMaxLoc(b_hist,&minValue, &maxValue, 0, 0);  

           Mat histImage(256, 256, CV_8UC3, Scalar(0, 0, 0));			 
			int hpt = saturate_cast<int>(0.9 * histSize);
			for(int i = 0; i < 256; i++)
			{
				float binValue = b_hist.at<float>(i);          
				int realValue = saturate_cast<int>(binValue * hpt/maxValue);
				rectangle(histImage,Point(i, histSize - 1), Point((i+1) - 1, histSize - realValue), Scalar(255));
			}
#if 0
			//printf("%d",b_hist.channels());
			int  hist_h = 400;
					int hist_w = 512;
					int bin_w = hist_w / histSize;
					Mat histImage(hist_w, hist_h, CV_8UC3, Scalar(0, 0, 0));
					normalize(b_hist, b_hist, 0, hist_h, NORM_MINMAX, -1, Mat());
					normalize(g_hist, g_hist, 0, hist_h, NORM_MINMAX, -1, Mat());
					normalize(r_hist, r_hist, 0, hist_h, NORM_MINMAX, -1, Mat());

			//render histogram chart  在直方图画布上画出直方图
			for (int i = 0; i < histSize; i++)
			{
				line(histImage, Point((i - 1)*bin_w, hist_h - cvRound(b_hist.at<float>(i - 1))), 
					Point((i)*bin_w, hist_h - cvRound(b_hist.at<float>(i))), Scalar(255, 0, 0), 2, LINE_AA);
			//	line(histImage, Point((i - 1)*bin_w, hist_h - cvRound(g_hist.at<float>(i - 1))),
			//		Point((i)*bin_w, hist_h - cvRound(g_hist.at<float>(i))), Scalar(0, 255, 0), 2, LINE_AA);
			//	line(histImage, Point((i - 1)*bin_w, hist_h - cvRound(r_hist.at<float>(i - 1))),
			//		Point((i)*bin_w, hist_h - cvRound(r_hist.at<float>(i))), Scalar(0, 0, 255), 2, LINE_AA);
			}
#endif
			imshow(OUTPUT_TITLE, histImage);
			waitKey(0);
			return 0;
}