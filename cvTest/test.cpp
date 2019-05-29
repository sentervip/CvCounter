#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <string>
#include <strstream>
#include <vector>
//#include <windows.h>
using namespace cv;
using namespace std;


//-----------------------------------【宏定义部分】--------------------------------------------
//		描述：定义一些辅助宏
//------------------------------------------------------------------------------------------------
#define WINDOW_NAME1 "src"					//为窗口标题定义的宏
#define WINDOW_NAME2 "split"        //为窗口标题定义的宏
#define WINDOW_NAME3 "dst"      //中间调试窗口
#define WINDOW_WIDTH   1920/2
#define WINDOW_HEIHT   1080/2
#define TIMES_ERODE  30//30
#define SCALE      1
#define MAX_KERNEL_SIZE  50
//-----------------------------------【全局变量声明部分】--------------------------------------
//		描述：全局变量的声明
//-----------------------------------------------------------------------------------------------
Mat imgTest;
static Mat frame;
static Mat img2, g_srcImage;
static Mat  g_dstImage;
static Mat g_grayImage, gray2;
static Mat drawing;
static int g_nThresh = 150;
static int g_nMaxThresh = 255;
static RNG g_rng(12345);
static Mat g_cannyMat_output;
static vector<vector<Point> > g_vContours;
static vector<Vec4i> g_vHierarchy;
static int g_sum = 0;

//-----------------------------------【全局变量声明部分】--------------------------------------

static int g_nStructElementSize = 4; //初始内核值
static void on_ElementSizeChange(int, void *);//回调函数


string int2str(int n) {
    
    strstream ss;
    string s;
    ss << n;
    ss >> s;
    
    return s;
}
int sort_point(vector<Point2f> mc, int * valid, Mat &drawing)
{
    if (mc.size() <2)
        return -1;
    
    Point2f pt_tmp;
    int i = 0, j = 0;
    for (vector<cv::Point2f>::iterator iter = mc.begin(); iter != mc.end(); ++iter) {
        if (i>0)
            if (abs(iter->x - pt_tmp.x) < 5 && abs(iter->y - pt_tmp.y) < 5) {
                // mc.erase(iter);
                cout << "erase i: " << i << endl;   putText(drawing, int2str(i), pt_tmp, CV_FONT_HERSHEY_DUPLEX, 0.8f, CV_RGB(255, 0, 0));
                valid[i] = 0;
                j++;
            }
        i++;
        pt_tmp = *iter;
        
    }
    
    return (i - j);
}


void counter(void)
{
    int mc_valid[400] = { 1 };
    
    
    printf("g_nThresh=%d\n", g_nThresh);
    
    // 使用Canndy检测边缘
    g_cannyMat_output.zeros(g_cannyMat_output.rows, g_cannyMat_output.cols, CV_8U);
    Canny(gray2, g_cannyMat_output, 150, 150 * 2, 3);
    //    Mat draw22;
    //    resize(g_cannyMat_output, draw22, cvSize(WINDOW_WIDTH, WINDOW_HEIHT));
    //    imshow( WINDOW_NAME3, draw22 );
    
    // 找到轮廓
    findContours(g_cannyMat_output, g_vContours, g_vHierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    
    // 计算矩
    vector<Moments> mu(g_vContours.size());
    for (unsigned int i = 0; i < g_vContours.size(); i++)
    {
        mu[i] = moments(g_vContours[i], false);
    }
    
    //  计算中心矩
    vector<Point2f> mc(g_vContours.size());
    
    for (unsigned int i = 0; i < g_vContours.size(); i++)
    {
        mc[i] = Point2f(static_cast<float>(mu[i].m10 / mu[i].m00), static_cast<float>(mu[i].m01 / mu[i].m00));
    }
    
    // 绘制轮廓
    drawing = Mat::zeros(g_cannyMat_output.size(), CV_8UC3);
    Scalar color = CV_RGB(255, 255, 255);
    memset(mc_valid, 1, sizeof(mc_valid));
    g_sum = sort_point(mc, mc_valid, drawing);
    int j = 0;
    for (unsigned int i = 0; i< g_vContours.size(); i++)
    {
        if (mc_valid[i] > 0) {
            j++;
            //  j =i+1;
            drawContours(drawing, g_vContours, i, Scalar(0, 0, 255), 2, 8, g_vHierarchy, 0, Point());//绘制外层和内层轮廓
            circle(drawing, mc[i], 3, Scalar(0, 255, 0), -1, 0.1f, 0);//绘制圆
            //cout << "int2str: " << int2str(i) << " mc: " << mc[i] << endl;
            putText(drawing, int2str(j), mc[i], CV_FONT_HERSHEY_DUPLEX, 0.8f, color);
        }
    }
    //g_sum =j;
    
    putText(drawing, "Sum:", Point2f(0, 50), CV_FONT_HERSHEY_DUPLEX, 1.0f, color);
    putText(drawing, int2str(g_sum), Point2f(100, 50), CV_FONT_HERSHEY_DUPLEX, 1.0f, color);
    
    // 显示到窗口中
    Mat draw2;
    resize(drawing, draw2, cvSize(WINDOW_WIDTH, WINDOW_HEIHT));
    imshow(WINDOW_NAME3, draw2);
    
    // 通过m00计算轮廓面积并且和OpenCV函数比较
    //printf("\t 输出内容: 面积和轮廓长度\n");
    for (unsigned int i = 0; i< mc.size(); i++)
    {
        printf("valid=%d, >通过m00计算出轮廓[%d]的面积: (M_00) = %.2f \n OpenCV函数计算出的面积=%.2f , 长度: %.2f \n\n", mc_valid[i],i, mu[i].m00, contourArea(g_vContours[i]), arcLength( g_vContours[i], true ) );
        Scalar color = Scalar( g_rng.uniform(0, 255), g_rng.uniform(0,255), g_rng.uniform(0,255) );
        // drawContours( drawing, g_vContours, i, color, 2, 8, g_vHierarchy, 0, Point() );
        // circle( drawing, mc[i], 4, color, -1, 1, 0 );
    }
    
}

//-----------------------------【on_ElementSizeChange( )函数】-------------------------------------
//		描述：腐蚀和膨胀操作内核改变时的回调函数
//-----------------------------------------------------------------------------------------------------
void on_ElementSizeChange(int, void *)
{
    // 加载源图像,选择分离特征明显通道
    
    Mat g_srcImage;
    vector<Mat>   channel;
    Mat img1 = frame.clone();
    //Mat img1 = imread("ym1.jpg", 1);
    
    imwrite("/Users/tanyongzheng/Documents/svn-cvTest/cvTest/testAi.jpg",img1);
    resize(img1,g_srcImage,Size(1920,1080));
    split(g_srcImage, channel);
    g_grayImage = channel.at(2);
    
    // 把原图像转化成灰度图像并进行平滑
    //cvtColor( g_srcImage, g_grayImage, CV_BGR2GRAY );
    //blur( g_grayImage, g_grayImage, Size(3,3) );
    threshold(g_grayImage, g_grayImage, 150, 255, THRESH_BINARY);
    
    //进行腐蚀
    Mat element1 = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    erode(g_grayImage, g_grayImage, element1);
    erode(g_grayImage, g_grayImage, element1);
    erode(g_grayImage, g_grayImage, element1);
    erode(g_grayImage, g_grayImage, element1);
    erode(g_grayImage, g_grayImage, element1);
    
    
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * g_nStructElementSize + 1, 2 * g_nStructElementSize + 1));
    //进行腐蚀或膨胀操作
    int i = TIMES_ERODE;
    gray2.zeros(gray2.rows, gray2.cols, CV_32S);
    printf("g_nStructElementSize=%d\n", g_nStructElementSize);
    while (i--) {
        // erode(g_srcImage, g_grayImage, element);
        morphologyEx(g_grayImage, gray2, MORPH_OPEN, element);
    }
    
    //显示分离效果图
    Mat show2;
    resize(gray2, show2, cvSize(WINDOW_WIDTH, WINDOW_HEIHT));
    imshow(WINDOW_NAME2, show2);
    counter();
    
}
void on_mouse(int event, int x, int y, int flags, void* param) {
    
    switch (event) {
        case CV_EVENT_RBUTTONUP:
            on_ElementSizeChange(NULL, NULL);
            break;
        case CV_EVENT_LBUTTONUP:
            cout << "x: " << x << " y:" << y << endl;
            circle(drawing, Point2i(x * 2, y * 2), 6, CV_RGB(0xEE, 0XEE, 0), -1, 0.3f, 0);//绘制圆,注意窗口和图像比例2
            
            Mat sumRoi(drawing, Rect(0, 0, 200, 50));
            sumRoi.setTo(0);
            putText(drawing, "Sum:", Point2f(0, 50), CV_FONT_HERSHEY_DUPLEX, 1.0f, CV_RGB(255, 255, 255));
            putText(drawing, int2str(++g_sum), Point2f(100, 50), CV_FONT_HERSHEY_DUPLEX, 1.0f, CV_RGB(255, 255, 255));
            
            // 显示到窗口中
            Mat draw2;
            resize(drawing, draw2, cvSize(WINDOW_WIDTH, WINDOW_HEIHT));
            imshow(WINDOW_NAME3, draw2);
            break;
    }
}

int main_test1(int argc, char** argv)
{
    //Mat r,g,b;
    VideoCapture capture(0);
    //imgTest = imread("D:/opencv/2/x64/Debug/ym.jpg",1);
    
    if (!capture.isOpened()) { //判断能够打开摄像头
        cout << "can not open the camera" << endl;
        cin.get();
        exit(1);
    }
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    
    int count = 0;
    capture >> frame; //载入图像
    
    if (frame.empty()) { //判断图像是否载入
        cout << "can not load the frame and exit" << endl;
        exit(0);
    }
    cout << "w/h: " << frame.cols << "/" << frame.rows << endl;
    
    // 创建新窗口
    namedWindow(WINDOW_NAME1, CV_WINDOW_NORMAL);
    //namedWindow(WINDOW_NAME2, CV_WINDOW_AUTOSIZE);
    namedWindow(WINDOW_NAME3, CV_WINDOW_AUTOSIZE);
    createTrackbar("ajust", WINDOW_NAME3, &g_nStructElementSize, MAX_KERNEL_SIZE, on_ElementSizeChange);
    cvSetMouseCallback(WINDOW_NAME3, on_mouse, 0);
    while (1) {
        capture >> frame; //载入图像
        if (frame.empty()) { //判断图像是否载入
            cout << "can not load the frame" << endl;
            
        }
        else {
            
            imshow(WINDOW_NAME1, frame);
            char c = waitKey(60); //延时30毫秒
            if (c == 27) //按ESC键退出
                break;
        }
    }
    
    waitKey(0);
    return(0);
}
