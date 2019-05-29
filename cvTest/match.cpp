//#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdlib.h>
#include <stdio.h>

/*
 模板匹配法 --图片查找
 滑动窗口的原理
 用等大小的模板窗口在范围中进行滑动 然后查找匹配
 */



CvPoint getNextMinLoc(IplImage* result , int templatWidth,int templatHeight,double maxValIn , CvPoint lastLoc){
    
    int y,x;
    int startY,startX,endY,endX;
    
    //计算大矩形的左上角坐标
    startY = lastLoc.y - templatHeight;
    startX = lastLoc.x - templatWidth;
    
    //计算大矩形的右下角的坐标  大矩形的定义 可以看视频的演示
    endY = lastLoc.y + templatHeight;
    endX = lastLoc.x + templatWidth;
    
    //不允许矩形越界
    startY = startY < 0 ? 0 : startY;
    startX = startX < 0 ? 0 : startX;
    endY = endY > result->height-1 ? result->height-1 : endY;
    endX = endX > result->width - 1 ? result->width - 1 : endX;
    
    //将大矩形内部 赋值为最大值 使得 以后找的最小值 不会位于该区域  避免找到重叠的目标
    for(y=startY;y<endY;y++){
        for(x=startX;x<endX;x++){
            cvSetReal2D(result,y,x,maxValIn);
        }
    }
    
    
    double minVal,maxVal;
    CvPoint minLoc,maxLoc;
    
    //查找result中的最小值 及其所在坐标
    cvMinMaxLoc(result,&minVal,&maxVal,&minLoc,&maxLoc,NULL);
    
    return minLoc;
    
    
}


int main_match(int argc, char* argv[]){
    
    IplImage*src,*templat,*result,*show;
    int srcW,templatW,srcH,templatH,resultW,resultH;
    
    //加载源图像
    src = cvLoadImage("/Volumes/work/test/svn-cvTest/cvTest/2input.jpg" , CV_LOAD_IMAGE_GRAYSCALE);
    
    //加载用于显示结果的图像
    show = cvLoadImage("/Volumes/work/test/svn-cvTest/cvTest/2input.jpg" );
    
    //加载模板图像
    templat = cvLoadImage("/Volumes/work/test/svn-cvTest/cvTest/2tmp.jpg" , CV_LOAD_IMAGE_GRAYSCALE);
    
    if(!src || !templat){
        printf("打开图片失败");
        return 0;
    }
    
    srcW = src->width;
    srcH = src->height;
    
    templatW = templat->width;
    templatH = templat->height;
    
    if(srcW<templatW || srcH<templatH){
        printf("模板不能比原图小");
        return 0;
    }
    
    //计算结果矩阵的宽度和高度
    resultW = srcW - templatW + 1;
    resultH = srcH - templatH + 1;
    
    //创建存放结果的空间
    result = cvCreateImage(cvSize(resultW,resultH),32,1);
    
    double minVal,maxVal;
    CvPoint minLoc,maxLoc;
    
    //进行模板匹配
    cvMatchTemplate(src,templat,result,CV_TM_SQDIFF);
    cvNamedWindow("result");
    cvShowImage("result",result);
    std::cout<<"result:"<<std::endl;

    
    //第一次查找最小值  即找到第一个最像的目标
    cvMinMaxLoc(result,&minVal,&maxVal,&minLoc,&maxLoc,NULL);
    
    
    //绘制第一个查找结果到图像上
    cvRectangle(show,minLoc,cvPoint(minLoc.x+templat->width,minLoc.y+templat->height),CV_RGB(0,255,0),1);
    std::cout<<"\n minVal: "<<minVal<<std::endl;
    
    
    //查找第二个结果
    minLoc = getNextMinLoc( result , templat->width,templat->height,  maxVal ,  minLoc);
    //绘制第二个结果
    cvRectangle(show,minLoc,cvPoint(minLoc.x+templat->width,minLoc.y+templat->height),CV_RGB(0,255,0),1);
    std::cout<<"\n minVal: "<<minVal<<std::endl;
    
    //查找第3个结果
    minLoc = getNextMinLoc( result , templat->width,templat->height,  maxVal ,  minLoc);
    //绘制第3个结果
    cvRectangle(show,minLoc,cvPoint(minLoc.x+templat->width,minLoc.y+templat->height),CV_RGB(0,255,0),1);
     std::cout<<"\n minVal: "<<minVal<<std::endl;
    
    //查找第4个结果
    minLoc = getNextMinLoc( result , templat->width,templat->height,  maxVal ,  minLoc);
    //绘制第4个结果
    cvRectangle(show,minLoc,cvPoint(minLoc.x+templat->width,minLoc.y+templat->height),CV_RGB(0,255,0),1);
     std::cout<<"\n minVal: "<<minVal<<std::endl;
    
    //显示结果
    cvNamedWindow("dst");
    cvShowImage("dst",show);
    cvWaitKey(0);
    
    return 0;
}
