
#pragma once

#include <iostream>
#include "Object.h"

using namespace std;
using namespace cv;

class Draw
{
public:
    static void draw(const Mat& src);
    static void drawContours(const Mat& src, const vector<vector<cv::Point> >& contours, const vector<cv::Vec4i>& hierarchy);
    static void drawMSERs(const Mat& src, const vector<vector<cv::Point> >& mser_features);
    static void drawObjects(const Mat& src, vector<Object>& objects);
    static void drawGradients(const Mat& src, const Mat_<double>& gradients);
    static void drawGradients(const vector<Object>& objects, const Mat_<double>& gradients);
    static void drawEchars(const Mat& src, const vector<Object>& objects);
    static void drawGradientLine(const Mat& src, const vector<Object>& objects, double a, double b);
    static void drawSurroundings(const Mat& src, const vector<Object>& objects);
    
private:
    static Mat srcImage;
    static void drawImage(const Mat& src);
    static Scalar* colorOfRadian(double radian);

};

