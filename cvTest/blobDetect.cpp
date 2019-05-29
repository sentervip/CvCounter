#include <opencv2/opencv.hpp>
#include <vector>
#include <map>
#include <iostream>
using namespace cv;
using namespace std;


static void help()
{
    cout << "\n This program demonstrates how to use BLOB to detect and filter region \n"
    "Usage: \n"
    "  ./detect_blob <image1(../data/detect_blob.png as default)>\n"
    "Press a key when image window is active to change descriptor";
}

void to_string(String & result, float t)

{
    
    ostringstream oss;//创建一个流
    
    oss<<t;//把值传递如流中
    
    result=oss.str();//获取转换后的字符转并将其写入result
}
static String Legende(SimpleBlobDetector::Params &pAct)
{
    String s = "";
    String  sup;
    String  inf;
    
    if (pAct.filterByArea)
    {
        
        //ostringstream oss;
        
        to_string(inf,pAct.minArea);
        to_string(sup,pAct.maxArea);
        //String inf = static_cast<ostringstream*>(&(ostringstream() << pAct.minArea))->str();
        //String sup = static_cast<ostringstream*>(&(ostringstream() << pAct.maxArea))->str();
        
        s = " Area range [" + inf + " to  " + sup + "]";
    }
    if (pAct.filterByCircularity)
    {
        to_string(inf,pAct.minCircularity);
        to_string(sup,pAct.maxCircularity);
        //String inf = static_cast<ostringstream*>(&(ostringstream() << pAct.minCircularity))->str();
        //String sup = static_cast<ostringstream*>(&(ostringstream() << pAct.maxCircularity))->str();
        if (s.length() == 0)
            s = " Circularity range [" + inf + " to  " + sup + "]";
        else
            s += " AND Circularity range [" + inf + " to  " + sup + "]";
    }
    if (pAct.filterByColor)
    {
        float i = (float)pAct.blobColor;
         to_string(inf,i);
        //String inf = static_cast<ostringstream*>(&(ostringstream() << (int)pAct.blobColor))->str();
        if (s.length() == 0)
            s = " Blob color " + inf;
        else
            s += " AND Blob color " + inf;
    }
    if (pAct.filterByConvexity)
    {
        to_string(inf,pAct.minConvexity);
        to_string(sup,pAct.maxConvexity);
//        String inf = static_cast<ostringstream*>(&(ostringstream() << pAct.minConvexity))->str();
//        String sup = static_cast<ostringstream*>(&(ostringstream() << pAct.maxConvexity))->str();
        if (s.length() == 0)
            s = " Convexity range[" + inf + " to  " + sup + "]";
        else
            s += " AND  Convexity range[" + inf + " to  " + sup + "]";
    }
    if (pAct.filterByInertia)
    {
        to_string(inf,pAct.minInertiaRatio);
        to_string(sup,pAct.maxInertiaRatio);
        //String inf = static_cast<ostringstream*>(&(ostringstream() << pAct.minInertiaRatio))->str();
        //String sup = static_cast<ostringstream*>(&(ostringstream() << pAct.maxInertiaRatio))->str();
        if (s.length() == 0)
            s = " Inertia ratio range [" + inf + " to  " + sup + "]";
        else
            s += " AND  Inertia ratio range [" + inf + " to  " + sup + "]";
    }
    return s;
}


#define SCALE  1
int main(int argc, char *argv[])
{
    vector<String> fileName;
    Mat img(600, 800, CV_8UC1);
    
    cout<<"cv ver:"<<CV_VERSION<<endl;
    if (argc == 1)
    {
        fileName.push_back("/Volumes/work/test/cvTest/detect_blob.png");
    }
    else if (argc == 2)
    {
        fileName.push_back(argv[1]);
    }
    else
    {
        help();
        return(0);
    }
    Mat img1 = imread(fileName[0], IMREAD_COLOR);
    resize(img1,img,Size(img1.cols/SCALE,img1.rows/SCALE));
    if (img.rows*img.cols <= 0)
    {
        cout << "Image " << fileName[0] << " is empty or cannot be found\n";
        return(0);
    }
    
    SimpleBlobDetector::Params pDefaultBLOB;
    // This is default parameters for SimpleBlobDetector
    pDefaultBLOB.thresholdStep = 1;
    pDefaultBLOB.minThreshold = 50;//10;
    pDefaultBLOB.maxThreshold = 255;//220;
    pDefaultBLOB.minRepeatability = 10; //2
    pDefaultBLOB.minDistBetweenBlobs = 10;//10;
    pDefaultBLOB.filterByColor = true; //false
    pDefaultBLOB.blobColor = 5;
    pDefaultBLOB.filterByArea = true;
    pDefaultBLOB.minArea = 25;//25;
    pDefaultBLOB.maxArea = 50;//5000;
    pDefaultBLOB.filterByCircularity = true;
    pDefaultBLOB.minCircularity = 0.6f;
    pDefaultBLOB.maxCircularity = 1.0;
    pDefaultBLOB.filterByInertia = false;
    pDefaultBLOB.minInertiaRatio = 0.1f;
    pDefaultBLOB.maxInertiaRatio = (float)1e37;
    pDefaultBLOB.filterByConvexity = false;
    pDefaultBLOB.minConvexity = 0.95f;
    pDefaultBLOB.maxConvexity = (float)1e37;
    // Descriptor array for BLOB
    vector<String> typeDesc;
    // Param array for BLOB
    vector<SimpleBlobDetector::Params> pBLOB;
    vector<SimpleBlobDetector::Params>::iterator itBLOB;
    // Color palette
    vector< Vec3b >  palette;
    for (int i = 0; i<65536; i++)
    {
        palette.push_back(Vec3b((uchar)rand(), (uchar)rand(), (uchar)rand()));
    }
    help();
    
    
    // This descriptor are going to be detect and compute BLOBS with 6 differents params
    // Param for first BLOB detector we want all
    typeDesc.push_back("BLOB");    // see http://docs.opencv.org/trunk/d0/d7a/classcv_1_1SimpleBlobDetector.html
    pBLOB.push_back(pDefaultBLOB);
    pBLOB.back().filterByArea = true;
    pBLOB.back().minArea = 1;
    pBLOB.back().maxArea = float(img.rows*img.cols);
    // Param for second BLOB detector we want area between 500 and 2900 pixels
    typeDesc.push_back("BLOB");
    pBLOB.push_back(pDefaultBLOB);
    pBLOB.back().filterByArea = true;
    pBLOB.back().minArea = 500;
    pBLOB.back().maxArea = 2900;
    // Param for third BLOB detector we want only circular object
    typeDesc.push_back("BLOB");
    pBLOB.push_back(pDefaultBLOB);
    pBLOB.back().filterByCircularity = true;
    // Param for Fourth BLOB detector we want ratio inertia
    typeDesc.push_back("BLOB");
    pBLOB.push_back(pDefaultBLOB);
    pBLOB.back().filterByInertia = true;
    pBLOB.back().minInertiaRatio = 0;
    pBLOB.back().maxInertiaRatio = (float)0.2;
    // Param for fifth BLOB detector we want ratio inertia
    typeDesc.push_back("BLOB");
    pBLOB.push_back(pDefaultBLOB);
    pBLOB.back().filterByConvexity = true;
    pBLOB.back().minConvexity = 0.;
    pBLOB.back().maxConvexity = (float)0.9;
    // Param for six BLOB detector we want blob with gravity center color equal to 0 bug #4321 must be fixed
    typeDesc.push_back("BLOB");
    pBLOB.push_back(pDefaultBLOB);
    pBLOB.back().filterByColor = true;
    pBLOB.back().blobColor = 0;
    
    itBLOB = pBLOB.begin();
    vector<double> desMethCmp;
    Ptr<Feature2D> b;
    String label;
    
    // Descriptor loop
    vector<String>::iterator itDesc;
    for (itDesc = typeDesc.begin(); itDesc != typeDesc.end(); itDesc++)
    {
        vector<KeyPoint> keyImg1;
        if (*itDesc == "BLOB")
        {
            b = SimpleBlobDetector::create(*itBLOB);
            label = Legende(*itBLOB);
            itBLOB++;
        }
        try
        {
            // We can detect keypoint with detect method
            vector<KeyPoint>  keyImg;
            vector<Rect>  zone;
            vector<vector <Point> >  region;
            Mat     desc, result(img.rows, img.cols, CV_8UC3);
            if (b.dynamicCast<SimpleBlobDetector>() != NULL)
            {
                Ptr<SimpleBlobDetector> sbd = b.dynamicCast<SimpleBlobDetector>();
                sbd->detect(img, keyImg, Mat());
                drawKeypoints(img, keyImg, result);
                int i = 0;
                for (vector<KeyPoint>::iterator k = keyImg.begin(); k != keyImg.end(); k++, i++)
                    circle(result, k->pt, (int)k->size, palette[i % 65536]);
            }
            namedWindow(*itDesc + label, WINDOW_AUTOSIZE);
            imshow(*itDesc + label, result);
            imshow("Original", img);
            waitKey();
        }
        catch (Exception& e)
        {
            cout << "Feature : " << *itDesc << "\n";
            cout << e.msg << endl;
        }
    }
    return 0;
}
