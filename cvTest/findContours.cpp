//
//  test.cpp
//  cvTest
//
//  Created by HFY on 17/6/6.
//  Copyright © 2017年 HFY. All rights reserved.
//
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/nonfree/nonfree.hpp>
#include<opencv2/legacy/legacy.hpp>
#include <iostream>
using namespace cv;
using namespace std;

#define TRUE   1
#define FALSE  0
#define SCALE  7
//-----------------------------------【宏定义部分】--------------------------------------------
//		描述：定义一些辅助宏
//------------------------------------------------------------------------------------------------
#define WINDOW_NAME1 "【原始图窗口】"			//为窗口标题定义的宏
#define WINDOW_NAME2 "【轮廓图】"					//为窗口标题定义的宏


//-----------------------------------【全局变量声明部分】--------------------------------------
//		描述：全局变量的声明
//-----------------------------------------------------------------------------------------------
static Mat g_srcImage;
static Mat g_grayImage;
static int g_nThresh = 80;
static int g_nThresh_max = 255;
static RNG g_rng(12345);
static Mat g_cannyMat_output;
static vector<vector<Point>> g_vContours;
static vector<Vec4i> g_vHierarchy;

//
//-----------------------------------【全局函数声明部分】--------------------------------------
//		描述：全局函数的声明
//-----------------------------------------------------------------------------------------------
static void ShowHelpText( );
static void on_ThreshChange(int, void* );


//findContours
//-----------------------------------【main( )函数】--------------------------------------------
//		描述：控制台应用程序的入口函数，我们的程序从这里开始执行
//-----------------------------------------------------------------------------------------------
int main_findContours( int argc, char** argv )
{
    //【0】改变console字体颜色
    system("color 1F");
    
    //【0】显示欢迎和帮助文字
    ShowHelpText( );
    
    // 加载源图像
   Mat  img1 = imread( "/Volumes/work/test/cvTest/ym2.jpg", 1 );
    
    //Mat g_srcImage ;
    
    resize(img1,g_srcImage,Size(img1.cols/SCALE,img1.rows/SCALE));
    if(!g_srcImage.data ) { printf("读取图片错误，请确定目录下是否有imread函数指定的图片存在~！ \n"); return false; }
    
    // 转成灰度并模糊化降噪
    cvtColor( g_srcImage, g_grayImage, CV_BGR2GRAY );
    blur( g_grayImage, g_grayImage, Size(3,3) );
    
    // 创建窗口
    namedWindow( WINDOW_NAME1, CV_WINDOW_AUTOSIZE );
    imshow( WINDOW_NAME1, g_srcImage );
    
    //创建滚动条并初始化
    createTrackbar( "canny阈值", WINDOW_NAME1, &g_nThresh, g_nThresh_max, on_ThreshChange );
    on_ThreshChange( 0, 0 );
    
    waitKey(0);
    return(0);
}

//-----------------------------------【on_ThreshChange( )函数】------------------------------
//      描述：回调函数
//----------------------------------------------------------------------------------------------
void on_ThreshChange(int, void* )
{
    
    // 用Canny算子检测边缘
    Canny( g_grayImage, g_cannyMat_output, g_nThresh, g_nThresh*2, 3 );  cout<<"g_nThresh: "<<g_nThresh<<std::endl;
    
    // 寻找轮廓
    findContours( g_cannyMat_output, g_vContours, g_vHierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    // 绘出轮廓
    Mat drawing = Mat::zeros( g_cannyMat_output.size(), CV_8UC3 );
    
    for( int i = 0; i< g_vContours.size(); i++ )
    {
        //Scalar color = Scalar( g_rng.uniform(0, 255), g_rng.uniform(0,255), g_rng.uniform(0,255) );//任意值
        drawContours( drawing, g_vContours, i, Scalar(0,0,255), 2, 8, g_vHierarchy, 0, Point() );
    }
    
    // 显示效果图
    cout<<"counts: "<<g_vContours.size()<<endl;
    imshow( WINDOW_NAME2, drawing );
}


//-----------------------------------【ShowHelpText( )函数】----------------------------------
//      描述：输出一些帮助信息
//----------------------------------------------------------------------------------------------
static void ShowHelpText()
{
    //输出欢迎信息和OpenCV版本
    printf("\n\n\t\t\t非常感谢购买《OpenCV3编程入门》一书！\n");
    printf("\n\n\t\t\t此为本书OpenCV2版的第70个配套示例程序\n");
    printf("\n\n\t\t\t   当前使用的OpenCV版本为：" CV_VERSION );
    printf("\n\n  ----------------------------------------------------------------------------\n");
    
    //输出一些帮助信息  
    printf(   "\n\n\t欢迎来到【在图形中寻找轮廓】示例程序~\n\n");  
    printf(   "\n\n\t按键操作说明: \n\n"  
           "\t\t键盘按键任意键- 退出程序\n\n"  
           "\t\t滑动滚动条-改变阈值\n" );  
}  
