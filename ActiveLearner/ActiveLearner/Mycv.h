
//#ifndef __ActiveLearner__Mycv__
//#define __ActiveLearner__Mycv__

#pragma once

#include <iostream>
#include "Features.h"
#include "Object.h"
#include "MSERegion.h"
#include "Text.h"
#include "Draw.h"

using namespace std;
using namespace cv;

class Mycv
{
public:
    // コンストラクタ
    Mycv(const string& filepath);
    
    // Interface Methods
    void detector();
    
    // Image Processing Methods
    void grayscale(const Mat& src, Mat& dst);
    void unsharpMasking(const Mat& src, Mat& dst, float k);
    void canny(const Mat &src, Mat &dst);
    void contours(const Mat &src, vector<vector<cv::Point>> &contours, vector<cv::Vec4i>& hierarchy, int mode, int method);
    
    void MSERs(const Mat& src, vector<MSERegion>& msers);
    
    //
    void createObjects(const vector<vector<cv::Point>> &contours, vector<Object> &objects);
    void mergeApartContours(vector<Object>& objects, vector<MSERegion>& msers);
    
    // 
    vector<Object>  detectObjects();
    vector<Text>    detectTexts();
    
private:
    Mat srcImage;
    string filename;
    
    cv::Rect* intersect(const cv::Rect& rect1, const cv::Rect& rect2);
    
};

//#endif /* defined(__ActiveLearner__Mycv__) */