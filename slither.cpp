#include "slither.hpp"
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

#define  FL_MIN_LENGTH   ( (IMG_WIDTH*0.5) )  
#define  FL_MIN_AREA     800//400
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
#define  SURFACE_REFERENCE        1200  //参考满分面积值

vector<StrMonmentTag> g_MonmentTag;  //momentes struct
vector<vector<Point> > g_Contours;
vector<strAreaTag> g_AllArea,g_CandidateArea; //all area and area of lines
vector<strAreaTag> * pAreaTag = &g_AllArea;
int part = PART_WAIST;
int g_LastDepth = 0;

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
//sort: descending order for x axis
int SortPointXAo(Point a1, Point a2)
{
	return a1.x < a2.x;
}
//sort: descending order for y axis
int SortPointYAo(Point a1, Point a2)
{
	return a1.y < a2.y;
}
//sort: ascending order length of monmentTag
int SortAoLen(StrMonmentTag a1, StrMonmentTag a2)
{
	return a1.d > a2.d;
}
int SortAoYaxis(StrMonmentTag a1, StrMonmentTag a2)
{
	return a1.mc.y > a2.mc.y;
}
//sort: ascending order square
int SortAoSqa(StrMonmentTag a1, StrMonmentTag a2)
{
	return a1.s > a2.s;
}
int SortCmpUp(int a1, int a2)
{
	return a1 < a2;
}
int SortArea(strAreaTag a1, strAreaTag a2)
{
	return g_MonmentTag.at(a1.McIndex).s > g_MonmentTag.at(a2.McIndex).s;
}
int SortDoc(strAreaTag a1, strAreaTag a2)
{
	return a1.doc > a2.doc;
}
int SortUp(strAreaTag a1, strAreaTag a2)
{
	return 0;//a1.boxUp > a2.boxUp;
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
	if(_pts.size() <1){
		printf("%s, size：%d\n",__FUNCTION__, _pts.size());
		return -1;
	}else if(_pts.size() ==1){
		pts.push_back(cvRound(_pts[0][0]) );
		out.push_back(pts.at(0));
	    return 1;
	}else;

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
Mat polyfit(vector<Point>& in_point, int n)
{
	int size = in_point.size();
	//所求未知数个数
	int x_num = n + 1;
	//构造矩阵U和Y
	Mat mat_u(size, x_num, CV_64F);
	Mat mat_y(size, 1, CV_64F);

	for (int i = 0; i < mat_u.rows; ++i)
		for (int j = 0; j < mat_u.cols; ++j)
		{
			mat_u.at<double>(i, j) = pow((float)in_point[i].x, j);
		}

		for (int i = 0; i < mat_y.rows; ++i)
		{
			mat_y.at<double>(i, 0) = in_point[i].y;
		}

		//矩阵运算，获得系数矩阵K
		Mat mat_k(x_num, 1, CV_64F);
		mat_k = (mat_u.t()*mat_u).inv()*mat_u.t()*mat_y;
		cout << mat_k << endl;
		return mat_k;
}
void GetPtsEdges(vector<Point>& SrcContour, vector<Point>& DstEdges)
{
	int i =0;
	int j=0;

	std::sort(SrcContour.begin(),SrcContour.end(),SortPointXAo);
	DstEdges.push_back(SrcContour.at(0));
	for( i=0; i < SrcContour.size();i++){		
		if(SrcContour.at(i).x > DstEdges.at(j).x){
			DstEdges.push_back(SrcContour.at(i));
			j++;
		}else if( SrcContour.at(i).x == DstEdges.at(j).x ){
			if(DstEdges.at(j).y > SrcContour.at(i).y){
				DstEdges.at(j).y = SrcContour.at(i).y;
			}
		}else ;
	}
}
void GetEdgesYInfo(StrMonmentTag &mc, vector<Point>& DstEdges)
{
	int i =0;
	int j=0;
	int sum = 0;
	int size = DstEdges.size();

	if( &mc == NULL ||  size <= 5  ){
		printf("%s invalid arg,exit", __FUNCTION__);
		return;
	}
	std::sort(DstEdges.begin(),DstEdges.end(),SortPointYAo);
	for( i=0; i < size;i++){		
		if( i > (0.2*size) && i < (0.8*size) ){
			sum += DstEdges.at(i).y;
			j++;
		}
	}

	//get average depth 
	if(j > 0){
		mc.AveYAxis = sum/j;
	}else{
		mc.AveYAxis = 0;
	}
}

int main()
{
	char dist[40];
    int i,j,tmp=0,iRet = 0;
    unsigned int aver = 0;
	float depth;
	Point pt1, pt2;
	int mc_valid[300] = {1};
	RNG g_rng(12345);
	Mat g_cannyMat_output;
	vector<Vec4i> g_vHierarchy;
	vector<int> g_lineY;
	Mat DestRGB;
	Mat m_origin = imread("D:/prj/z1/src/img/slider/4.jpg", IMREAD_COLOR);
	Mat m_gray,m_grayEnhance;  
    std::vector<cv::Point> in_point, in;
    Point ipt;

	//s1 preprocess     
	int w = m_origin.cols;
	int h = m_origin.rows;
	if(m_origin.data == NULL){
		printf("can not open file\n");
		waitKey(0);
		getchar();
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

	_m_gray = _m_gray.mul(1.1);
	//GaussianBlur(_m_gray, m_gray, Size(3, 3) , 1.0); 
    GaussianBlur(_m_gray, _m_gray, Size(3, 3 ), 1.0);

    //morphology
	int g_nElementShape = MORPH_RECT;
	Mat element = getStructuringElement(g_nElementShape, Size(5, 5) );
	morphologyEx(_m_gray, m_gray, CV_MOP_ERODE, element);
	imwrite("2erode.jpg",m_gray);
	element = getStructuringElement(g_nElementShape, Size(3, 3));
	morphologyEx(m_gray, m_gray, CV_MOP_DILATE, element);
    imwrite("3dilate.jpg",m_gray);

	
	//Bin
	Mat m_bin;	
	threshold(m_gray, m_bin, 80, 255, CV_THRESH_OTSU);
	imwrite("4bin.jpg", m_bin);

	//s2 get contour
	Mat bMat = m_bin.clone();
	Canny( bMat, g_cannyMat_output, 50, 255);
	findContours(bMat, g_Contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	Mat markers = Mat::zeros(bMat.size(), CV_32SC1);

	for( i = 0; i < g_Contours.size(); i++)
		drawContours(markers, g_Contours, static_cast<int>(i), Scalar::all(static_cast<int>(i)+1), -1);

	circle(markers, Point(5, 5), 3, CV_RGB(255, 255, 255), -1);
	imwrite("5contours.jpg", markers);

	//s3 get center monments
	vector<Moments> mu(g_Contours.size() );
	for( i = 0; i < g_Contours.size(); i++)
	{ 
		mu[i] = moments( g_Contours[i], false ); 
	}

	//  calc mc
	vector<Point2f> mc( g_Contours.size() );
	for(  i = 0; i < g_Contours.size(); i++ )
	{
		mc[i] = Point2i( static_cast<float>(mu[i].m10/mu[i].m00), static_cast<float>(mu[i].m01/mu[i].m00 )) ;
	}

	// s4  update args of mc and drawing contour 
	Mat drawing = Mat::zeros( g_cannyMat_output.size(), CV_8UC3 );
	Scalar color = CV_RGB(0,255,0);
	StrMonmentTag mcTag;
	int d,s;
	g_MonmentTag.clear();
    for( i = 0; i< g_Contours.size(); i++ ){	
		mcTag.PtsEdges.clear();
        d =(long int) arcLength( g_Contours[i], true );
        s =(int) contourArea(g_Contours[i]);
		mcTag.s = s;		
        switch (part) {
            case PART_WAIST:
                 if (d < FL_MIN_LENGTH || mcTag.s < FL_MIN_AREA || mc.at(i).y <= (MIN_IMG_Y + 15)) {
                     continue;
                 }
                 break;
            case PART_ARM:
            case PART_THIGH:
                 if (d < FL_MIN_LENGTH || mcTag.s < FL_MIN_AREA || mc.at(i).y <= MIN_IMG_Y) {
                    continue;
                 }
                break;
            default:
                if (d < FL_MIN_LENGTH || mcTag.s < FL_MIN_AREA || mc.at(i).y <= (MIN_IMG_Y + 15)) {
                    continue;
                }
                break;
        }
		mcTag.mc = mc.at(i);
		mcTag.contourIndex = i;
		GetPtsEdges(g_Contours[mcTag.contourIndex],mcTag.PtsEdges);
		mcTag.d = abs(mcTag.PtsEdges.at(mcTag.PtsEdges.size()-1).x - mcTag.PtsEdges.at(0).x );
		if(mcTag.d < IMG_WIDTH/2){
	        continue;
		}
		GetEdgesYInfo(mcTag, mcTag.PtsEdges);
        printf("[%d],mc.x=%f,mc.y=%f, s:%d ,d:%d,y=%d\n", i,mc[i].x,mc[i].y, (int)mcTag.s, (int)mcTag.d, mcTag.AveYAxis);
		drawContours( drawing, g_Contours, i, color, 2, 8, g_vHierarchy, 0, Point() );     
		circle( drawing, mc[i], 3, Scalar(0,0,255), -1, 0.1f, 0 );
		putText(drawing,int2str(i,mcTag.s,mcTag.d),mc[i],CV_FONT_HERSHEY_DUPLEX,0.3f,Scalar(255,255,255));
		putText(drawing,int2str(mc[i].x, mc[i].y),Point2i(mc[i].x-20,mc[i].y+10),CV_FONT_HERSHEY_DUPLEX,  0.3f,Scalar(0,0,255));		
		g_MonmentTag.push_back(mcTag);
	}
	imwrite("6mc.jpg", drawing);
	if(g_MonmentTag.size() <= 0){
		printf("no valid g_MonmentTag");
		getchar();
		return ENULL_WAVE;
	}

	//s5 get DOC
	i = 0;
	strAreaTag area;
	float ScoreS,ScoreLen;
	std::vector<StrMonmentTag> StrMcSortAoLen(g_MonmentTag.size());
	std::copy(g_MonmentTag.begin(),g_MonmentTag.end(),StrMcSortAoLen.begin());
	std::sort(StrMcSortAoLen.begin(),StrMcSortAoLen.end(),SortAoLen);
	for( vector<StrMonmentTag>::iterator it = StrMcSortAoLen.begin(); it != StrMcSortAoLen.end(); i++){
       area.McIndex = i;
       switch(part) {
           case PART_WAIST:
               if (StrMcSortAoLen.at(i).mc.y > 80) {
                   area.dls = std::exp(-0.005 * StrMcSortAoLen.at(i).mc.y) * 139.45; // y = 139.45(e^-0.005x)
               } else {
		           area.dls = 71.348 * pow((int)StrMcSortAoLen.at(i).mc.y, 0.0803); //y = 71.348 x^0.0803
               }
               break;
           case PART_ARM:
               area.dls = -30.03* std::log((float)StrMcSortAoLen.at(i).mc.y) +190.08; //Y= -30.03ln(x)+190.08  
               break;
           default:
               area.dls = -30.03* std::log((float)StrMcSortAoLen.at(i).mc.y) +190.08; //Y= -30.03ln(x)+190.08  
               break;
       }
	   ScoreLen = StrMcSortAoLen.at(i).d *100/IMG_WIDTH; // score of area's length
	   ScoreS  = (StrMcSortAoLen.at(i).s *100)/ SURFACE_REFERENCE;
	   ScoreS > 100? ScoreS=100:ScoreS;

	   //len/surface/y axis
	   //doc =  (lenRate + s/SURFACE* lenRate) * dls
	   area.doc = (ScoreLen + ScoreS) * (area.dls/100);
	   g_CandidateArea.push_back(area);
	   it++;
	}

	//s6 choose destArea
	uint32_t DestLine = 0;
	if(g_CandidateArea.size() >=2 ) {
        std::sort(g_CandidateArea.begin(), g_CandidateArea.end(), SortDoc);
		DestLine = g_CandidateArea.at(0).McIndex; 
		
		//resolve limit max data: 在存在多区域情况下，超过预设部位阈值 & 第二个比第一小50pixls以上; & doc>100 | g_last-now <15;则取第二个区域
		switch(part){
		case PART_WAIST: 
			 tmp = LIMIT_WAIST; break;
		case PART_ARM: 
			tmp = LIMIT_ARM; break;
		default: tmp = LIMIT_ARM; break;
		}
        if ( StrMcSortAoLen.at(DestLine).mc.y > tmp && (abs(StrMcSortAoLen.at(g_CandidateArea.at(1).McIndex).mc.y - StrMcSortAoLen.at(DestLine).mc.y) > 50) ) {
            if(g_CandidateArea.at(g_CandidateArea.at(1).McIndex).doc > 60 || (abs(g_LastDepth - StrMcSortAoLen.at(g_CandidateArea.at(1).McIndex).mc.y) < 15) ){
			    DestLine = g_CandidateArea.at(g_CandidateArea.at(1).McIndex).McIndex;            
			}
         }
		 
	}else if(g_CandidateArea.size() == 0){
	        printf("no valid g_CandidateArea");
		    getchar();
	        return ENULL_WAVE;
	}else{
		DestLine = g_CandidateArea.at(0).McIndex;  
	}
	//get points on up-edges 	
	in_point = StrMcSortAoLen.at(DestLine).PtsEdges;
	std::sort(in_point.begin(),in_point.end(),SortPointXAo);
	in.push_back(in_point.at(0));
	j = 0;
	for(int i=0; i < in_point.size();i++){		
		if(in_point.at(i).x > in.at(j).x){
			in.push_back(in_point.at(i));
			j++;
		}else if( in_point.at(i).x == in.at(j).x ){
			if(in.at(j).y > in_point.at(i).y){
			   in.at(j).y = in_point.at(i).y;
			}
		}else ;
	}
#if 0
	//insert start and end points
	vector<Point>::iterator it = in.begin(); 
	if(in.at(0).x != 1){
		it = in.insert(it,Point(0,in.at(0).y));
	}
	if(in.at(in.size()-1).x != IMG_WIDTH){
		in.push_back( Point(IMG_WIDTH-1,in.at(in.size()-1).y));
	}
#endif
	//计算结果可视化
	cv::cvtColor(_m_gray,DestRGB,COLOR_GRAY2RGB);

#if 0
    //n:多项式阶次
	int n = 9;
	Mat mat_k = polyfit(in, n);


	//计算结果可视化
	//Mat out(400, 500, CV_8UC3,Scalar::all(0));
	Mat out;
	cv::cvtColor(_m_gray,out,COLOR_GRAY2RGB);
	

	//画出拟合曲线
	
	for ( i = 0; i < in.size(); ++i)
	{	
		ipt.x = in[i].x;
		ipt.y = 0;
		for ( j = 0; j < n + 1; ++j)
		{
			ipt.y += mat_k.at<double>(j, 0)*pow((float)ipt.x,j);
		}
		//circle(out, ipt, 1, Scalar(0, 0, 255), CV_FILLED, CV_AA);
		
	}
#endif

	//画出原始散点
	aver = 0;
	for (i = 0; i < in.size(); i++)
	{
		ipt = in[i];
        if(i>=15 && i < (in.size()-15)) {
            aver = (unsigned int) in[i].y + aver;
        }
		circle(DestRGB, ipt, 2, Scalar(0X8B, 0xEC, 0xFF), CV_FILLED, CV_AA); //FFEC8B yellow
        //printf("av=%d,i=%d,in.y=%d",aver,i,in[i].y);
	}
	aver /= (i-30);
	g_LastDepth = aver;
	memset(dist,0,sizeof(dist));
	printf("pix:%d", aver);
    sprintf(dist," %d", aver);
   // putText(DestRGB,dist,Point2i(in.size()/2+5,aver/2),CV_FONT_HERSHEY_DUPLEX,0.4f,Scalar(0,0,255));
   // cv::arrowedLine(DestRGB,Point(in.size()/2,0),Point(in.size()/2,in.at(in.size()/2).y),Scalar(0,0,255),1);
	imshow("N=9", DestRGB);
	//imwrite("DestRGB.jpg",DestRGB);
	waitKey(0);
	getchar();
	return 0;
}

