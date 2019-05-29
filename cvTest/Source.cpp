
#include"opencv2/opencv.hpp" 
#include"fstream"

#include"math.h"

using namespace cv;
using namespace std;

int main_source()
{
	Mat _img = imread("/Users/tanyongzheng/Documents/svn-cvTest/cvTest/p2.jpg", 1);
    Mat img;
    
    GaussianBlur(_img, img, Size(5, 5), 0.5);
    
    int width = img.cols;
    int height = img.rows;
    Mat gray = Mat::zeros(img.size(),CV_8UC1);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int pix = img.at<Vec3b>(i, j)[0] + img.at<Vec3b>(i, j)[2] - img.at<Vec3b>(i, j)[1];
            if (pix > 255)
                pix = 255;
            if (pix < 0)
                pix = 0;
            gray.at<uchar>(i, j) = (uchar)pix;
        }
    }
    //imwrite("4.jpg",gray);
    Mat bw;
    threshold(gray, bw, 0, 255, CV_THRESH_BINARY);
    //imwrite("5.jpg", bw);
    
    Mat distanceImg(img.size(), CV_32FC1);
    distanceTransform(bw, distanceImg, CV_DIST_L2, 3);
    float maxValue = 0;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            float pix = distanceImg.at<float>(i, j);
            if (pix > maxValue)
                maxValue = pix;
        }
    }
    
    maxValue = 0.55 * maxValue;
    Mat dist = Mat::zeros(img.size(), CV_8UC1);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            float pix = distanceImg.at<float>(i, j);
            if (pix > maxValue)
                dist.at<uchar>(i, j) = 255;
        }
    }
    imwrite("6.jpg", dist);
    
    vector<vector<Point> > contours;
    findContours(dist, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    
    int num = contours.size();
    for (int i = 0; i < num; i++)
    {
        if (contours.size() < 5)
        {
            if (num>0)
                num--;
        }
    }
    
    //    cout << "num:" << num << endl;
    
    
    Mat markers = Mat::zeros(dist.size(), CV_32SC1);
    
    for (size_t i = 0; i < contours.size(); i++)
        drawContours(markers, contours, static_cast<int>(i), Scalar::all(static_cast<int>(i)+1), -1);
    
    circle(markers, Point(5, 5), 3, CV_RGB(255, 255, 255), -1);
    
    watershed(img, markers);
    Mat mark = Mat::zeros(markers.size(), CV_8UC1);
    markers.convertTo(mark, CV_8UC1);
    bitwise_not(mark, mark);
    
    vector<Vec3b> colors;
    for (size_t i = 0; i < contours.size(); i++)
    {
        int b = theRNG().uniform(0, 255);
        int g = theRNG().uniform(0, 255);
        int r = theRNG().uniform(0, 255);
        colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
    }
    
    
    Mat dst = Mat::zeros(markers.size(), CV_8UC3);
    
    
    for (int i = 0; i < markers.rows; i++)
    {
        for (int j = 0; j < markers.cols; j++)
        {
            int index = markers.at<int>(i, j);
            
            if (index > 0 && index <= static_cast<int>(contours.size()))
                dst.at<Vec3b>(i, j) = colors[index - 1];
            else
                dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
        }
    }
    
        imwrite("/Users/tanyongzheng/Documents/svn-cvTest/cvTest/7.jpg",dst);
    //    imshow("Final Result", dst);


	waitKey();
	return 0;
}
