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
    
    vector<cv::Point> surroundings;
    vector<cv::Point> contourPixels;
    vector<cv::Point> corrPairPixels;
    vector<double> thetas;
    vector<double> thetasWtoB;
    vector<double> thetasBtoW;
    vector<double> corrThetas;
    vector<double> surrThetas;
    
    bool isPositive = true;
    double Gangle, Fcorr, Vwidth;
    double Echar;
    vector<double> features;
    
    bool grouped;
    vector<Object> neighbors;
    double neighborDistance;
    
    
#pragma mark -
#pragma mark Inclusion Relationship Methods
public:
    static int isLeftLarge(Object obj1, Object obj2) {
        return (obj1.rectArea > obj2.rectArea);
    }
    bool isParentOf(Object obj);
    bool isChildOf(Object obj);
    void mergeObject(Object obj);
    
#pragma mark -
#pragma mark Compute Properties Methods
private:
    void computeProperties();
    void computeColor();
    void computeContourArea(int gridSize);
    void computeTLRB();
    void uniqueContour();
    
};