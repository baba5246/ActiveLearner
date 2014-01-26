
#pragma once

#include <iostream>
#include <sstream>
#include "Object.h"
#include "Text.h"
#include "MSERegion.h"
#include "Sample.h"
#include "SWTObject.h"

using namespace std;
using namespace cv;

class Draw
{
public:
    static void draw(const Mat& src);
    static void draw(const Mat& src1, const Mat& src2);
    static void drawGrays(const Mat& r, const Mat& g, const Mat& b);
    static void drawEdges(const Mat& r, const Mat& g, const Mat& b);
    static Mat drawContours(const Mat& src, const vector<vector<cv::Point> >& contours, const vector<cv::Vec4i>& hierarchy);
    static Mat drawMSERs(const Mat& src, const vector<vector<cv::Point> >& mser_features);
    static Mat drawMSERs(const Mat& src, const vector<MSERegion>& msers);
    static Mat drawObjects(const Mat& src, const vector<Object*>& objects);
    static Mat drawGradients(const Mat& src, const Mat_<double>& gradients);
    static Mat drawGradients(const vector<Object*>& objects, const Mat_<double>& gradients);
    static Mat drawEchars(const Mat& src, const vector<Object*>& objects);
    static Mat drawGradientLine(const Mat& src, const vector<Object*>& objects, double a, double b);
    static Mat drawSurroundings(const Mat& src, const vector<Object*>& objects);
    static Mat drawText(const Mat& src, Text*& text);
    static Mat drawText(const Mat& src, Text*& text, cv::Rect small, cv::Rect large);
    static Mat drawTexts(const Mat& src, const vector<Text*>& texts);
    static Mat drawSWT(const Mat& swt, const double max);
    static Mat drawSWTObjects(const Mat_<double>& swt,
                               const vector<SWTObject>& swtobjects);
    static Mat drawSamples(const Mat& src, const vector<Sample>& samples);
    static Mat drawLabeles(const Mat& label);
    static Mat drawSWTComponents(const Mat& swt,
                                 const vector<vector<cv::Point> >& components);
    static Mat drawInnerAreaOfObjects(const Mat& swt, const vector<Object*>& objects);
    
private:
    static Mat srcImage;
    static void drawImage(const Mat& src);
    static void drawImage(const Mat& src1, const Mat& src2);
    static Scalar* colorOfRadian(double radian);
    static Scalar colorWithCount(const int count);
    
};

