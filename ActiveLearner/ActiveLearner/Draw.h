
#pragma once

#include <iostream>
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
    static void drawGrays(const Mat& r, const Mat& g, const Mat& b);
    static void drawEdges(const Mat& r, const Mat& g, const Mat& b);
    static void drawContours(const Mat& src, const vector<vector<cv::Point> >& contours, const vector<cv::Vec4i>& hierarchy);
    static void drawMSERs(const Mat& src, const vector<vector<cv::Point> >& mser_features);
    static void drawMSERs(const Mat& src, const vector<MSERegion>& msers);
    static void drawObjects(const Mat& src, vector<Object*>& objects);
    static void drawGradients(const Mat& src, const Mat_<double>& gradients);
    static void drawGradients(const vector<Object*>& objects, const Mat_<double>& gradients);
    static void drawEchars(const Mat& src, const vector<Object*>& objects);
    static void drawGradientLine(const Mat& src, const vector<Object*>& objects, double a, double b);
    static void drawSurroundings(const Mat& src, const vector<Object*>& objects);
    static void drawText(const Mat& src, Text*& text);
    static void drawTexts(const Mat& src, const vector<Text*>& texts);
    static void drawSWT(const Mat& swt, const double max);
    static void drawSWTObjects(const Mat_<double>& swt,
                               const vector<SWTObject>& swtobjects);
    static void drawSamples(const Mat& src, const vector<Sample>& samples);
    static void drawLabeles(const Mat& label);
    static void drawSWTComponents(const Mat& swt,
                                  const vector<vector<cv::Point> >& components);
    
private:
    static Mat srcImage;
    static void drawImage(const Mat& src);
    static Scalar* colorOfRadian(double radian);
    static Scalar colorWithCount(const int count);

};

