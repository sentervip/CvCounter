#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <string>
#include <strstream>
#include <vector>
using namespace cv;
using namespace std;


//-----------------------------------【宏定义部分】--------------------------------------------
//		描述：定义一些辅助宏
//------------------------------------------------------------------------------------------------
#define WINDOW_NAME1 "old"					//为窗口标题定义的宏
#define WINDOW_NAME2 "update"        //为窗口标题定义的宏
#define WINDOW_NAME3 "midTest"      //中间调试窗口
#define WINDOW_WIDTH   658*2
#define WINDOW_HEIHT   370*2

#define SCALE      1
//-----------------------------------【全局变量声明部分】--------------------------------------
//		描述：全局变量的声明
//-----------------------------------------------------------------------------------------------
static Mat g_srcImage;
static Mat g_grayImage;
static int g_nThresh = 150;
static int g_nMaxThresh = 255;
static RNG g_rng(12345);
static Mat g_cannyMat_output;
static vector<vector<Point> > g_vContours;
static vector<Vec4i> g_vHierarchy;

//-----------------------------------【全局变量声明部分】--------------------------------------
//		描述：全局变量的声明
//-----------------------------------------------------------------------------------------------
static void on_ThreshChange(int, void* );

//aizj add
static Mat  g_dstImage;//原始图和效果图
static int g_nTrackbarNumer = 0;//0表示腐蚀erode, 1表示膨胀dilate
static int g_nStructElementSize = 11; //结构元素(内核矩阵)寸
#define TIMES_ERODE       50

static void Process();//膨胀和腐蚀的处理函数
static void on_TrackbarNumChange(int, void *);//回调函数
static void on_ElementSizeChange(int, void *);//回调函数


int main1( int argc, char** argv )
{
    
    // 加载源图像
    Mat  img1 = imread( "/Volumes/work/test/svn-cvTest/cvTest/ym1.jpg", 0 );
    
    //Mat g_srcImage ;
    
    resize(img1,g_srcImage,Size(1920,1080));
    
    // 把原图像转化成灰度图像并进行平滑
    //cvtColor( g_srcImage, g_grayImage, CV_BGR2GRAY );
    blur( g_srcImage, g_grayImage, Size(3,3) );
    
    // 创建新窗口
    namedWindow( WINDOW_NAME1, CV_WINDOW_AUTOSIZE );
    namedWindow(WINDOW_NAME3,  CV_WINDOW_AUTOSIZE);
    Mat src2;
    resize(g_srcImage, src2, cvSize(WINDOW_WIDTH, WINDOW_HEIHT));
    imshow( WINDOW_NAME1, src2 );
    
    //add by aizj
    threshold(g_srcImage, g_srcImage, 150,255, 0);
    
    //获取自定义核
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2*g_nStructElementSize+1, 2*g_nStructElementSize+1),Point( g_nStructElementSize, g_nStructElementSize ));
    erode(g_srcImage, g_srcImage, element);
    
    //创建轨迹条
    // createTrackbar("dilate/erode", WINDOW_NAME1, &g_nTrackbarNumer, 1, on_TrackbarNumChange);
    createTrackbar("kernelSize", WINDOW_NAME1, &g_nStructElementSize, 21, on_ElementSizeChange);
    //end aizj
    
    //创建滚动条并进行初始化
    createTrackbar( "thresh", WINDOW_NAME1, &g_nThresh, g_nMaxThresh, on_ThreshChange );
    //on_ThreshChange( 0, 0 );
    
    //等待按键
    
    waitKey(0);
    return(0);
}
string int2str(int n) {
    
    strstream ss;
    string s;
    ss << n;
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
        if(i>0)
            if( abs(iter->x - pt_tmp.x) < 1 || abs(iter->y - pt_tmp.y) < 1 ){
                // mc.erase(iter);
                cout<<"erase i: "<<i<<endl;   putText(drawing,int2str(i),pt_tmp,CV_FONT_HERSHEY_DUPLEX,0.8f,CV_RGB(255,0,0));
                valid[i] = 0;
                j++;
            }
        i++;
        pt_tmp = *iter;
        
    }
    
    return (i - j+1);
}



//-----------------------------------【on_ThreshChange( )函数】-------------------------------
//		描述：回调函数
//-----------------------------------------------------------------------------------------------
void on_ThreshChange(int, void* )
{
    int mc_valid[300] = {1};
    
    
    printf("g_nThresh=%d\n",g_nThresh);
    // 使用Canndy检测边缘
    Canny( g_grayImage, g_cannyMat_output, g_nThresh, g_nThresh*2, 3 );
    //imshow(WINDOW_NAME3, g_cannyMat_output);
    
    // 找到轮廓
    findContours( g_cannyMat_output, g_vContours, g_vHierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    // 计算矩
    vector<Moments> mu(g_vContours.size() );
    for(unsigned int i = 0; i < g_vContours.size(); i++ )
    { mu[i] = moments( g_vContours[i], false ); }
    
    //  计算中心矩
    vector<Point2f> mc( g_vContours.size() );
    
    for( unsigned int i = 0; i < g_vContours.size(); i++ )
    {
        mc[i] = Point2f( static_cast<float>(mu[i].m10/mu[i].m00), static_cast<float>(mu[i].m01/mu[i].m00 ));
    }
    
    // 绘制轮廓
    Mat drawing = Mat::zeros( g_cannyMat_output.size(), CV_8UC3 );

    

    Scalar color = CV_RGB(255,255,255);
    memset(mc_valid, 1, sizeof(mc_valid));
    int sum = g_vContours.size();//sort_point(mc, mc_valid,drawing);
    int j =0;
    for( unsigned int i = 0; i< g_vContours.size(); i++ )
    {
        //Scalar color = Scalar( g_rng.uniform(0, 255), g_rng.uniform(0,255), g_rng.uniform(0,255) );//随机生成颜色值
       // if(mc_valid[i] > 0  ){
          //  j++;
            j =i+1;
            drawContours( drawing, g_vContours, i, Scalar(0,0,255), 2, 8, g_vHierarchy, 0, Point() );//绘制外层和内层轮廓
            circle( drawing, mc[i], 3, Scalar(0,255,0), -1, 0.1f, 0 );//绘制圆
            cout<<"int2str: "<<int2str(i)<<" mc: "<<mc[i]<<endl;
            putText(drawing,int2str(j),mc[i],CV_FONT_HERSHEY_DUPLEX,0.8f,color);
        //}
     }
    
    putText(drawing,"Sum:",Point2f(0,50),CV_FONT_HERSHEY_DUPLEX,1.0f,color);
    putText(drawing,int2str(sum),Point2f(100,50),CV_FONT_HERSHEY_DUPLEX,1.0f,color);
    
    // 显示到窗口中
    Mat draw2;
    namedWindow( WINDOW_NAME2, CV_WINDOW_AUTOSIZE );
    resize(drawing, draw2, cvSize(WINDOW_WIDTH, WINDOW_HEIHT));
    imshow( WINDOW_NAME2, draw2 );
    
    // 通过m00计算轮廓面积并且和OpenCV函数比较
    printf("\t 输出内容: 面积和轮廓长度\n");
    for(unsigned  int i = 0; i< mc.size(); i++ )
    {
        printf("valid=%d, >通过m00计算出轮廓[%d]的面积: (M_00) = %.2f \n OpenCV函数计算出的面积=%.2f , 长度: %.2f \n\n", mc_valid[i],i, mu[i].m00, contourArea(g_vContours[i]), arcLength( g_vContours[i], true ) );
       // Scalar color = Scalar( g_rng.uniform(0, 255), g_rng.uniform(0,255), g_rng.uniform(0,255) );
       // drawContours( drawing, g_vContours, i, color, 2, 8, g_vHierarchy, 0, Point() );
       // circle( drawing, mc[i], 4, color, -1, 1, 0 );
        
    }
    
}
//-----------------------------【Process( )函数】------------------------------------
//		描述：进行自定义的腐蚀和膨胀操作
//-----------------------------------------------------------------------------------------
void Process()
{
    //获取自定义核
    //Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2*g_nStructElementSize+1, 2*g_nStructElementSize+1),Point( g_nStructElementSize, g_nStructElementSize ));
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2*g_nStructElementSize+1, 2*g_nStructElementSize+1));
    
    //进行腐蚀或膨胀操作
    int i= TIMES_ERODE;
    printf("g_nStructElementSize=%d\n",g_nStructElementSize);
    while (i--) {

            //erode(g_srcImage, g_grayImage, element);
            morphologyEx(g_srcImage, g_grayImage, MORPH_OPEN, element);

            }
    
    //显示效果图
    Mat g_gray2;
    resize(g_grayImage, g_gray2, cvSize(WINDOW_WIDTH, WINDOW_HEIHT));
    imshow(WINDOW_NAME2, g_gray2);
}

//-----------------------------【on_ElementSizeChange( )函数】-------------------------------------
//		描述：腐蚀和膨胀操作内核改变时的回调函数
//-----------------------------------------------------------------------------------------------------
void on_ElementSizeChange(int, void *)
{
    //内核尺寸已改变，回调函数体内需调用一次Process函数，使改变后的效果立即生效并显示出来
    Process();
}
