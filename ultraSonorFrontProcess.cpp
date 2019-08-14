#include "counter.hpp"
#include "fstream"
#include <iostream>
#include <string>
#include <strstream>
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> 
#include "line.h"
#include "getEnvelop.hpp"

#define  POINT_NUM        4096 //2584
#define  DOWN_RATE      8

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
/**
read US dataline from fpga and process to text and show to matlab 
*/
int main()
{
    int i,j,k,iRet = 0;
    int mc_valid[300] = {1};
    RNG g_rng(12345);
    Mat g_cannyMat_output;
    vector<Vec4i> g_vHierarchy;
    Mat img ;
	env_detect* m_envlop = new env_detect();
    
	Mat m_line = Mat(POINT_NUM,1,CV_16SC1,iDataLine);
	Mat m_lineEnvelop = Mat(POINT_NUM,1,CV_16SC1);
	Mat m_pryDown = Mat(POINT_NUM/DOWN_RATE,1,CV_16SC1);
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
	
	//s3 get envelop and normalize
    fstream envFile("D:/prj/z1/src/CvCounter/3envelop.txt",ios::out|ios::trunc);
	if(!envFile.is_open()){
	    printf("error: can not open 3envelop.txt\n");
		envFile.close();
		return -2;
	}
	memset(str,0,sizeof(str));
	short t1,t2;
	m_envlop->env_half((short *)m_line.data,(short *)m_lineEnvelop.data,POINT_NUM);
	cv::normalize(m_lineEnvelop,m_lineEnvelop,0,255,cv::NORM_MINMAX);
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
	
	//s4 prydown4
	fstream downFile("D:/prj/z1/src/CvCounter/4downFile.txt",ios::out|ios::trunc);
	if(!downFile.is_open()){
	    printf("error: can not open downFile.txt\n");
		downFile.close();
		return -2;
	}
	memset(str,0,sizeof(str));
	//cv::pyrDown(m_lineEnvelop,m_pryDown,cv::Size(m_pryDown.cols/4,m_pryDown.rows));	
	for( i=0; i<m_lineEnvelop.cols;i++){
		for( j=0; j<m_lineEnvelop.rows; j+=DOWN_RATE){
			if(j>=DOWN_RATE){
				iRet = 0;
				for(k=0;k<DOWN_RATE;k++){
				    iRet += m_lineEnvelop.at<short>(j-k,i);
				}
				m_pryDown.at<short>(j/DOWN_RATE,i) = iRet/DOWN_RATE;
			}else if(0==i && 0==j){
				m_pryDown.at<short>(0,0) = m_lineEnvelop.at<short>(0,0);
			}else{;}
			sprintf(str,"%d,",m_pryDown.at<short>(j/DOWN_RATE,i));
			downFile.write(str, strlen(str) );
		}
	}
	downFile.close();
	waitKey();
    return 0;
}
