#pragma once

#include <iostream>

using namespace std;
using namespace cv;

class Object
{
public:
    Object();
    Object(const string& filepath, const vector<cv::Point>& contour);
    ~Object();
    
#pragma mark -
#pragma Attribute Variables
public:
    string filename;
    
    cv::Point origin;
    cv::Point tp, lp, rp, bp;
    cv::Size size;
    int width, height;
    cv::Point centroid;
    double aspectRatio;
    cv::Rect rect;
    int rectArea;
    int contourArea;
    Scalar color;
    
    int mserIndex;
    
    double rectRatio;
    int nearestIndex;

    vector<int> parents;
    vector<int> children;
    
    vector<cv::Point> contourPixels;
    vector<cv::Point> corrPairPixels;
    vector<double> thetas;
    vector<double> thetasWtoB;
    vector<double> thetasBtoW;
    vector<double> corrThetas;
    
    bool isPositive = true;
    double Gangle, Fcorr, Vwidth;
    double Echar;
    vector<double> features;
    
    bool grouped;
    vector<Object> neighbors;
    double neighborDistance;
    
    
#pragma mark -
#pragma mark Inclusion Relationship Methods
    static bool IsLeftLarge(Object obj1, Object obj2);
    bool isParentOf(Object obj);
    bool isChildOf(Object obj);
    void mergeObject(Object obj);
    
private:
    
#pragma mark -
#pragma mark Compute Properties Methods
    void computeProperties();
    void computeColor();
    void computeContourArea(int gridSize);
    void computeTLRB();
    
};