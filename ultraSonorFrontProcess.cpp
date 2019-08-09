#include "counter.hpp"
#include "fstream"
#include <iostream>
#include <string>
#include <strstream>
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include<algorithm> 
#include "line.h"
#include "getEnvelop.hpp"

#define  FL_MIN_LENGTH   200
#define  FL_MIN_AREA     400
#define  DELTA           (6)  // contour error   1/6 *w
#define  DELTA_POINT     (10)  // point error  1/10 *w
#define  SUM_CALC_POINT   (100)
//#define  MIN_X           200
#define  MIN_Y           300
#define  MIN_CANDIDAT_DIAMETER    (200)
#define  MIN_CANDIDAT_AREA        (1000)
#define  POINT_NUM        2584
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

int main()
{
    int i,j,iRet = 0;
    int mc_valid[300] = {1};
    RNG g_rng(12345);
    Mat g_cannyMat_output;
    vector<Vec4i> g_vHierarchy;
    Mat img ;//=imread("D:/prj/z1/src/CvCounter/zjY.png", 1);
    //imshow("OriImg", img);
	env_detect* m_envlop = new env_detect();
    
	Mat m_line = Mat(POINT_NUM,1,CV_16SC1,iDataLine);
	Mat m_lineEnvelop = Mat(POINT_NUM,1,CV_16SC1);
	//memcpy(g_line.data,(char*)&iDataLine,2584*2);  // int64 maybe not ok
	printf("[0]=%d,[1000]=%d,[2000]=%d,[2583]=%d\n", m_line.at<short>(0,0),m_line.at<short>(1000,0),m_line.at<short>(2000,0),m_line.at<short>(2583,0));
	//printf("[0]=%d,[100]\n", g_line.at<short>(1000,0));
	/** copy to img
	Mat m_Fpga(2584,640,CV_16SC1);
	for(i=0; i<640;i++){
		m_line.copyTo(m_Fpga.colRange(i,i+1)); //error overflow;
		//printf("[0][%d]=%d,[1000]=%d,[2000]=%d,[2583]=%d\n",i, m_Fpga.at<short>(0,i),m_Fpga.at<short>(1000,i),m_Fpga.at<short>(2000,i),m_Fpga.at<short>(2583,i));
	}
	*/
	//s2 blur
    GaussianBlur(m_line, m_line, Size(15, 15), 1, 1);
	//printf("cmp：[100]%d:%d, [1000]%d:%d", m_Fpga.at<short>(100,100),img.at<short>(100,100), m_Fpga.at<short>(1000,400),img.at<short>(1000,400));
    int width = m_line.cols;
    int height = m_line.rows;
    imwrite("1gray.jpg",m_line);

    //s2 abs
	fstream absFile("D:/prj/z1/src/CvCounter/2abs.txt",ios::out|ios::trunc);
	if(!absFile.is_open()){
	    printf("error: can not open 2abs.txt\n");
		absFile.close();
		return -2;
	}
	char str[20]={0,0};
	for( i=0; i<height ;i++){
		for( j=0; j<width; j++){
			if(m_line.at<short>(i,j) < 0){
				m_line.at<short>(i,j) = - m_line.at<short>(i,j);
			}else{
				iRet++;
			}
			sprintf(str,"%d,",m_line.at<short>(i,j));
			//absFile<<m_line.at<short>(i,j)<<",";
			absFile.write(str, strlen(str) );
		}
	}
	absFile.close();
	
	//s3 get envelop
    fstream envFile("D:/prj/z1/src/CvCounter/3envelop.txt",ios::out|ios::trunc);
	if(!envFile.is_open()){
	    printf("error: can not open 3envelop.txt\n");
		envFile.close();
		return -2;
	}
	memset(str,0,sizeof(str));
	short t1,t2;
	m_envlop->env_half((short *)m_line.data,(short *)m_lineEnvelop.data,POINT_NUM);

	//s4 blur and save
	GaussianBlur(m_lineEnvelop, m_lineEnvelop, Size(15, 15), 1, 1);
	for( i=0; i<height ;i++){
		for( j=0; j<width; j++){
			//t1 = m_line.at<short>(i,j) ;
			//m_lineEnvelop.at<short>(i,j) = m_envlop->env_half1(t1);
			
			sprintf(str,"%d,",m_lineEnvelop.at<short>(i,j));
			envFile.write(str, strlen(str) );
		}
	}
	envFile.close();
	waitKey();
    return 0;
}
