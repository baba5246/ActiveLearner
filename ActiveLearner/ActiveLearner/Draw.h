
//#ifndef __ActiveLearner__Draw__
//#define __ActiveLearner__Draw__

#pragma once

#include <iostream>
#include "Object.h"

using namespace std;
using namespace cv;

class Draw
{
public:
    static void draw(const Mat& src);
    static void drawContours(const Mat& src, const vector<vector<cv::Point>>& contours, const vector<cv::Vec4i>& hierarchy);
    static void drawMSERs(const Mat& src, const vector<vector<cv::Point>>& mser_features);
    static void drawObjects(const Mat& src, const vector<Object> objects);
    
private:
    static Mat srcImage;
    static void drawImage(const Mat& src);
};


//#endif /* defined(__ActiveLearner__Draw__) */
