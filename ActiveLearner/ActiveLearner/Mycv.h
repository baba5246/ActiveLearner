#pragma once

#include <iostream>
#include <sstream>
#include "MSERegion.h"
#include "Draw.h"

using namespace std;
using namespace cv;

class Mycv
{
public:
    // コンストラクタ
    Mycv(Mat& image);
    ~Mycv();
    
    // Interface Methods
    void detector(vector<Object>& objects);
    
    // Image Processing Methods
    void grayscale(const Mat& src, Mat& dst);
    void unsharpMasking(const Mat& src, Mat& dst, float k);
    void canny(const Mat &src, Mat &dst);
    void contours(const Mat &src, vector<vector<cv::Point> > &contours, vector<cv::Vec4i>& hierarchy, int mode, int method);
    void sobelFiltering(const Mat& graySrc, Mat_<double>& gradients);
    void MSERs(const Mat& src, vector<MSERegion>& msers);
    
private:
    Mat srcImage;
    IplImage srcIplImage;
    
};
