//
//  main.cpp
//  cvTest
//
//  Created by HFY on 17/5/11.
//  Copyright © 2017年 HFY. All rights reserved.
//

#include <stdio.h>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "opencv/cxcore.h"
#include "opencv/cv.h"
#include "opencv/cvaux.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"        // Basic OpenCV structures (cv::Mat)
#include "opencv2/core/core_c.h"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/features2d/features2d.hpp"
//#include "opencv2/ml/ml.hpp"
//#include "opencv2/core/base.hpp"
//#include "opencv2/core/cuda.hpp"
#include   "opencv2/highgui/highgui.hpp"


using namespace std;
using namespace cv;

#define CV_TEST_FILENAME_0     "/Users/HFY/Documents/grass468X700.jpg"
#define CV_TEST_FILENAME_1     "/Users/HFY/Documents/lena-1.jpg"
#define IMG_RGB24_DIR          "/Users/HFY/Documents/girl1920X1200.jpg"

#define CV_TEST_IMAGE_WIDTH          468
#define CV_TEST_IMAGE_HEIGHT         700
#define CV_TEST_IMAGE_SIZE           (CV_TEST_IMAGE_WIDTH*CV_TEST_IMAGE_HEIGHT)

#define IMG_RGB24_WIDTH                   1920
#define IMG_RGB24_HEIGH                   1200
#define IMG_RGB24_SIZE               (IMG_RGB24_WIDTH * IMG_RGB24_HEIGH)

int saveFs(char *pDir,char * pFrame,int len)
{
    // save jpg
    fstream f2(pDir, ios::out | ios::binary);
    int i;
    for(i =0;i<(len/300);i++){
        f2.write((const char *) (pFrame + i * 300), 300);
    }
    f2.close();
    return 0;
    // imwrite(img1_dir, raw_yuv);
}

int main1(int argc, char** argv)
{
    unsigned char * ptr_raw;
    unsigned char * ptr_out;
    unsigned char * ptr_tmp;
    FILE * fp;
    clock_t t1,t2;
    
    
    
    //AmbaKAL_TaskSleep(30000);
    ///////////////////// init ///////////////////////////////
    cout<<"Amba_OpenCVTest start...\n";
    
    ///////////////// Mat operate ///////////////////////////////
    Mat mat=Mat(1920,1080,CV_32F,100.5);
    Mat mat1=Mat(1920,1080,CV_32F,200.5);
    Mat tempmat;
    mat.copyTo(tempmat);
    Mat roi(mat,Rect(0,0,1080,720));
    Mat roi1(mat,Rect(0,0,1080,720));


    for(int i=0;i<1080;i++)
        for(int j=0;j<720;j++){
            roi.at<float>(i,j) *= roi1.at<uchar>(i,j);
        }
    //mat-mat1µƒ1∑∂ ˝
    norm(mat,mat1,CV_L1);
    //mat-mat1µƒ2∑∂ ˝
    norm(mat,mat1,CV_L2);
    //¥Ú”°matµƒƒ⁄»›
    //roi = eyemat(Range(0,500),Range(2,500));
    roi.mul(roi1);
    roi.inv();
    //cout<<"roi[200,100]=%f\n",roi.at<float>(0,100));
    cout<<"end Mat\n";
    
    /////////////////canny///////////////////////////////
    ptr_raw = (unsigned char *) malloc(CV_TEST_IMAGE_SIZE*3);
    if(ptr_raw == NULL)
    {
        cout<<"ambamalloc error!\n";
        return -1;
    }
    ptr_out = (unsigned char *) malloc(CV_TEST_IMAGE_SIZE*3);
    if(ptr_out == NULL)
    {
        cout<<"ambamalloc error!\n";
        return -1;
    }
    
    
    Mat cay_out;
    Mat raw_yuv =imread(CV_TEST_FILENAME_0);
//    imshow("lena", raw_yuv);
//    cvWaitKey();
    cout<<"canny start\n";
    resize(raw_yuv, raw_yuv, Size2i(256,256));
    t1 = clock();

    for(int i=0;i<100;i++){
        Canny(raw_yuv, cay_out, 50, 150, 3);
        resize(cay_out, cay_out, Size(128, 128));
    }
    t2 = clock();
    cout<<"t1:"<<t1<<" t2:"<<t2<<" t2-t1:"<<t2-t1<<endl;
    cout<<"canny end 100\n";
    ptr_tmp = cay_out.ptr<uchar>(0, 0);
    fp = fopen(CV_TEST_FILENAME_1, "wb");
    if(fp == NULL)
    {
        cout<<"fopen1 error!\n";
        if(ptr_raw != NULL)
            free(ptr_raw);
        if(ptr_out != NULL)
            free(ptr_out);
        return -1;
    }
    fwrite(ptr_tmp, CV_TEST_IMAGE_SIZE/4, 1, fp);
    fclose(fp);
    if(ptr_raw != NULL)
        free(ptr_raw);
    
    if(ptr_out != NULL)
        free(ptr_out);
    
    cout<<"Amba cany end\n";
    
    //////////////////////////// hog //////////////////
    unsigned char * pRGB24_raw = (unsigned char *) malloc(IMG_RGB24_SIZE*3);
    if(pRGB24_raw == NULL)
    {
        cout<<"ambamalloc rgb24 error!\n";
        return -1;
    }
    FILE* fp2 = fopen(IMG_RGB24_DIR, "rb");
    if(fp2 == NULL)
    {
        cout<<"AmbaFS_fopen2 error!\n";
        return -1;
    }
    int len = fread(pRGB24_raw, 3, IMG_RGB24_SIZE, fp2);	cout<<"3,len:"<<len<<endl;
    fclose(fp2);
    int x=320,y=180,w=70,h=80;
    int error_code = 0;
    Mat rgb24(IMG_RGB24_HEIGH,IMG_RGB24_WIDTH,CV_8UC3,pRGB24_raw);
    
    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    vector<Rect> found;
    hog.detectMultiScale(rgb24.clone(), found, 0, Size(8,8), Size(32,32), 1.05, 2);
    cout<<"end hog\n";
    
    ///////////////////////// calculate histogram /////////////////////////////////
    /// ∑÷∏Ó≥…3∏ˆµ•Õ®µ¿ÕºœÒ ( R, G ∫Õ B )
    vector<Mat> rgb_planes;
    split( rgb24.clone(), rgb_planes );
    int histSize = 255;
    
    /// …Ë∂®»°÷µ∑∂Œß ( R,G,B) )
    float range[] = { 0, 255 } ;
    const float* histRange = { range };
    bool uniform = true; bool accumulate = false;
    Mat r_hist, g_hist, b_hist;
    
    /// º∆À„÷±∑ΩÕº:
    calcHist( &rgb_planes[0], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );
    calcHist( &rgb_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
    calcHist( &rgb_planes[2], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
    cout<<"end calcHist\n";
    
    /////////////////////////////fast detect////////////////////
    //Mat dest;
    // vector of keyPoints  
    //std::vector<KeyPoint> keyPoints;
    
    //FastFeatureDetector fast(40);// detecte 40 
    // feature point detection  
    //fast.detect(rgb24,keyPoints); 
    //drawKeypoints(rgb24.clone(),keyPoints, dest, Scalar::all(-1), DrawMatchesFlags::DRAW_OVER_OUTIMG);
    //cout<<"end fast\n",len);
    
    
    
    
    if(pRGB24_raw != NULL)
        free(pRGB24_raw);
    
    cout<<"end all\n";
    cvWaitKey(0);
    return 0;
}

//#include <iostream>
//#include <opencv/highgui.h>
//#include <opencv/cv.h>

int main(int argc, char** argv)

{
    
    cvNamedWindow("Image", CV_WINDOW_AUTOSIZE);
    
    //IplImage *img=cvLoadImage("/Users/HFY/Documents/grass468X700.jpg", CV_LOAD_IMAGE_ANYCOLOR);
   // Mat img = imread("/Users/HFY/Documents/grass468X700.jpg");
    
   // imshow("img", img);
    
   // cvShowImage("image", img);
    
    VideoCapture capture(0);
    while(1){
        Mat frame;
        capture >> frame;
        imshow("video",frame);
        waitKey(5);
    }
    cvWaitKey(30);
    
   // cvReleaseImage(&img);
   
    cvDestroyWindow("image");
    
    return 0;
    
}
