#pragma once

#include <iostream>
#include <sstream>
#include "MSERegion.h"
#include "Draw.h"

using namespace std;
using namespace cv;

enum MYCV_GRAY { MYCV_GRAY_B = 0, MYCV_GRAY_G = 1, MYCV_GRAY_R = 2};

class Mycv
{
public:
    // コンストラクタ
    Mycv(Mat& image);
    ~Mycv();
    
    // オペレータ
    Mycv & operator= (const Mycv & other)
    {
        if (this!=&other) {
            Mat new_src(other.srcImage);
            srcImage = new_src;
            IplImage new_srcIpl(other.srcIplImage);
            srcIplImage = new_srcIpl;
        }
        return *this;
    }
    
    
    // Interface Methods
    void detector(vector<Object>& objects);
    
    // Image Processing Methods
    void grayscale(const Mat& src, Mat& dst);
    void grayscale(const Mat &src, Mat &dst, int type);
    void unsharpMasking(const Mat& src, Mat& dst, float k);
    void canny(const Mat &src, Mat &dst);
    void mergeEdges(const Mat &r, const Mat &g, const Mat &b, Mat &dst);
    void contours(const Mat &src, vector<vector<cv::Point> > &contours, vector<cv::Vec4i>& hierarchy, int mode, int method);
    void sobelFiltering(const Mat& graySrc, Mat_<double>& gradients);
    void MSERs(const Mat& src, vector<MSERegion>& msers);
    void decreaseColorsWithKmeans(const Mat& src, Mat& dst);
    clock_t SWTMinus(const Mat& edge, const Mat& gradient, Mat& swt);
    clock_t SWTPlus(const Mat& edge, const Mat& gradient, Mat& swt);
    clock_t SWTComponents(const Mat& swt, vector<vector<cv::Point> >& components);
//    void decreaseColorsWithMedian(const Mat& src, Mat& dst);
    
private:
    Mat srcImage;
    IplImage srcIplImage;
    
    //ラベルのバッファ長
    //予想領域数より十分に大きく
    const int BUF_LABEL=512;
};
