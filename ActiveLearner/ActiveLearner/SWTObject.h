#pragma once

#include <iostream>

using namespace std;
using namespace cv;

class SWTObject {
    
public:
    
    SWTObject();
    SWTObject(vector<cv::Point> region, const Mat& swt);
    
    Mat swt;
    vector<cv::Point> region;
    cv::Rect rect;
    cv::RotatedRect rotatedRect;
    cv::Point centroid;
    int width = 0, height = 0;
    double aspectRatio = 0, areaRatio = 0;
    double mean = 0, variance = 0;
    Scalar color;
    
private:
    
    void computeProperties();
    void computeMeanAndVariance();
};