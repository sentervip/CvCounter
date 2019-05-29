#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
using namespace cv;
using namespace std;

#define MAX_NUM 500  // 预期最大细胞数量
#define WIDTH 640  //图片宽度
#define HEIGHT 480   //图片高度

int label[HEIGHT][WIDTH];  //每个像素点的标记，其大小代表细胞序号
uchar pixel[HEIGHT][WIDTH];  //记录每个像素点的灰度值
int area[MAX_NUM],perimeter[MAX_NUM];  // 记录每个细胞的面积和周长
int flag_cell[MAX_NUM];   //标记每个细胞是否完整
float degree[MAX_NUM];  //记录每个细胞的密集度

int peried[MAX_NUM];     //标记某个细胞是否被计算过周长，0表示没有
int beginPeried[2];   //记录开始数周长的初始点坐标(i,j)



void DoLabel(int i, int j, int counter)
{
    if(pixel[i][j] >  210) return;                  //把一些较亮的点也算进来
    if(label[i][j] == 0)
    {
        label[i][j] = counter;
        area[counter]++;
    }
    else return;
    
    if(i > 0) DoLabel(i-1,j,counter);
    if(j > 0) DoLabel(i,j-1,counter);
    if(i < HEIGHT-1) DoLabel(i+1,j,counter);
    if(j < WIDTH-1) DoLabel(i,j+1,counter);
    
    return;
}

int next(int i, int j, int b)
{
    if(b == 0)
    {
        if(j>0)
        {
            if(label[i][j-1]) return 0;
        }
        if(i<HEIGHT-1)
        {
            if(label[i+1][j]) return 1;
        }
        if(j<WIDTH-1)
        {
            if(label[i][j+1]) return 2;
        }
        if(i>0)
        {
            if(label[i-1][j]) return 3;
        }
    }
    else if(b == 1)
    {
        if(i<HEIGHT-1)
        {
            if(label[i+1][j]) return 1;
        }
        if(j<WIDTH-1)
        {
            if(label[i][j+1]) return 2;
        }
        if(i>0)
        {
            if(label[i-1][j]) return 3;
        }
        if(j>0)
        {
            if(label[i][j-1]) return 0;
        }
    }
    else if(b == 2)
    {
        if(j<WIDTH-1)
        {
            if(label[i][j+1]) return 2;
        }
        if(i>0)
        {
            if(label[i-1][j]) return 3;
        }
        if(j>0)
        {
            if(label[i][j-1]) return 0;
        }
        if(i<HEIGHT-1)
        {
            if(label[i+1][j]) return 1;
        }
    }
    else if(b == 3)
    {
        if(i>0)
        {
            if(label[i-1][j]) return 3;
        }
        if(j>0)
        {
            if(label[i][j-1]) return 0;
        }
        if(i<HEIGHT-1)
        {
            if(label[i+1][j]) return 1;
        }
        if(j<WIDTH-1)
        {
            if(label[i][j+1]) return 2;
        }
    }else{
        ;
    }
    return 4;
}

void DoPerimeter( int i,int j,int b,int tmp)
{
    int f;
    int t = next(i,j,b);
    f = (t+3)%4;
    
    switch(t)
    {
        case 0:
            if(i!=beginPeried[0] || j-1!= beginPeried[1])    // 判断是否与起始点重合
            {
                perimeter[tmp]++;
                DoPerimeter(i, j-1, f, tmp);
            }
            break;
        case 1:
            if(i+1!=beginPeried[0] || j!=beginPeried[1])
            {
                perimeter[tmp]++;
                DoPerimeter(i+1, j, f, tmp);
            }
            break;
        case 2:
            if(i!=beginPeried[0] || j+1!=beginPeried[1])
            {
                perimeter[tmp]++;
                DoPerimeter(i, j+1, f, tmp);
            }
            break;
        case 3:
            if(i-1!=beginPeried[0] || j!=beginPeried[1])
            {
                perimeter[tmp]++;
                DoPerimeter(i-1, j, f, tmp);
            }
            break;
        default:
            break;
    }
}


int find_max(int first,int last)
{
    int i,max;
    
    max=first;
    for(i=first; i<=last; i++)
    {
        if(flag_cell[i]==1)
            if(area[i]>area[max])
                max=i;
    }
    return max;
}

int find_min(int first,int last)
{
    int i,min;
    
    for(i=1;i<last;i++)
        if(flag_cell[i]==1)
        {
            min=i;
            break;
        }
    for(i=first; i<=last; i++)
    {
        if(flag_cell[i]==1)
            if(area[i]<area[min])
                min=i;
    }
    return min;
}

int find_center_x(int cell)
{
    long sum_x=0;
    int center_x;
    
    for(int i=0; i<HEIGHT; i++)
        for(int j=0; j<WIDTH; j++)
        {
            if(label[i][j] == cell)
            {
                sum_x+=i;
            }
        }
    center_x=sum_x/area[cell];
    return center_x;
}

int find_center_y(int cell)
{
    long sum_y=0;
    int center_y;
    
    for(int i=0; i<HEIGHT; i++)
        for(int j=0; j<WIDTH; j++)
        {
            if(label[i][j] == cell)
            {
                sum_y+=j;
            }
        }
    center_y=sum_y/area[cell];
    return center_y;
}
int main( )
{
    int cell_num = 0;  // 记录细胞数量
    int counter = 0;   // 标签计数
    int max;  //最大的细胞的编号
    int min;  //最小的细胞的编号
    
    float average_degree=0;  //平均密集度
    float sum_degree=0;
    float max_cell_degree,min_cell_degree;  //最大细胞和最小细胞的密集度
    long sum_area=0;  //所有细胞的面积总和
    long sum_perimeter=0;  //所有细胞的周长总和
    int average_area;  //平均面积
    int average_perimeter;  //平均周长
    
    int max_cell_center_x,max_cell_center_y;  //最大细胞的中心点坐标
    int min_cell_center_x,min_cell_center_y;  //最小细胞的中心点坐标
    
    void DoLabel(int i, int j, int counter);  //标记每个像素的细胞编号
    void DoPerimeter( int i,int j,int b,int tmp);    // 计算一个细胞的周长,递归函数
    int find_max(int first,int last);       // 查找面积最大的细胞
    int find_min(int first,int last);       // 查找面积最小的细胞
    int find_center_x(int cell);    //查找细胞中点横坐标
    int find_center_y(int cell);    //查找细胞中点纵坐标
    
    
   // IplImage* image = cvLoadImage("/Volumes/work/test/cvTest/cell.bmp",0) ;   // 以灰度方式打开图像
    Mat  img1 = imread("/Volumes/work/test/cvTest/blk.bmp",0);
    Mat  image2;
    cv::resize(img1,image2,cvSize(WIDTH,HEIGHT));
    
    //if(!image) return -1;
    //Mat* image2 = image;       //灰度图片
    //cvSaveImage("cell2.bmp",image2);
    
    for(int i=0; i<HEIGHT; i++)
        for(int j=0; j<WIDTH; j++)
        {
            label[i][j] = 0;
            //uchar* pix = CV_IMAGE_ELEM(image2,uchar,i,j);
            //pixel[i][j] = *pix;
            pixel[i][j] = image2.at<uchar>(i,j);
        }
    
    for(int i=0; i<HEIGHT; i++)
        for(int j=0; j<WIDTH; j++)
        {
            if(pixel[i][j] <170  && !label[i][j])
            {
                counter++;
                DoLabel(i,j,counter);
            }
            
        }
    cell_num = counter;
    
    for(int i=0; i<HEIGHT; i++)               //寻找边界点
        for(int j=0; j<WIDTH; j++)
        {
            int tmp = label[i][j];
            if (tmp != 0)
            {
                if(!peried[tmp])
                {
                    beginPeried[0] = i;
                    beginPeried[1] = j;
                    DoPerimeter( i, j, 0, tmp);
                    peried[tmp] = 1;
                }
            }
        }
    
    for(int c = 1; c <= cell_num; c++)
    {
        if(area[c]<400)
        {
            flag_cell[c]=0;
            counter--;
        }
        else flag_cell[c]=1;
    }
    
    max = find_max(1,cell_num);
    min = find_min(1,cell_num);
    max_cell_center_x = find_center_x(max);
    max_cell_center_y = find_center_y(max);
    min_cell_center_x = find_center_x(min);
    min_cell_center_y = find_center_y(min);
    
    for(int c = 1; c <= cell_num; c++)
    {
        if(flag_cell[c]==1)
        {
            degree[c] =(float)area[c]/(perimeter[c]*perimeter[c]);
            sum_degree+=degree[c];
            sum_area+=area[c];
            sum_perimeter+=perimeter[c];
        }
    }
    
    cell_num=counter;
    average_degree=sum_degree/cell_num;
    average_area=sum_area/cell_num;
    average_perimeter=sum_perimeter/cell_num;
    max_cell_degree=(float)area[max]/(perimeter[max]*perimeter[max]);
    min_cell_degree=(float)area[min]/(perimeter[min]*perimeter[min]);
    
    printf("the cell number is : %d\n",cell_num);
    printf("\nthe biggest cell :\n");
    printf("      center:( %d , %d )\n",max_cell_center_x,max_cell_center_y);
    printf("      the area is : %d\n",area[max]);
    printf("      the perimeter is : %d\n",perimeter[max]);
    printf("      the degree is : %f\n",max_cell_degree);
    
    printf("\nthe smallest cell :\n");
    printf("      center:( %d , %d )\n",min_cell_center_x,min_cell_center_y);
    printf("      the area is : %d\n",area[min]);
    printf("      the perimeter is : %d\n",perimeter[min]);
    printf("      the degree is : %f\n",min_cell_degree);
    
    printf("\nthe average degree is : %f\n",average_degree);
    printf("the average area of all cells is : %d\n",average_area);
    printf("the average perimeter of all cells is : %d\n",average_perimeter);
    
    for(int i=0; i<HEIGHT; i++)
        for(int j=0; j<WIDTH; j++)
        {
            //uchar* pix = &CV_IMAGE_ELEM(image2,uchar,i,j);
            
            if(label[i][j]==max||label[i][j]==min)
                //*pix=0;
                image2.at<uchar>(i,j) = 0;
        }
    imwrite("cell3.bmp",image2);
    cvNamedWindow( "test", 1 );
    imshow( "test", image2 );
    cvWaitKey();
    cvDestroyWindow( "test" );
    return 0;
}
