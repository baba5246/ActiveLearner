#pragma once

#include <iostream>

using namespace std;
using namespace cv;

class Object
{
public:
    
    Object(const string& filepath, const vector<cv::Point>& contour);
    ~Object();
    
    string filename;
    
    cv::Point origin;
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
    
    double Gangle, Fcorr, Vwidth;
    double Echar;
    vector<double> features;
    
    bool grouped;
    vector<Object> neighbors;
    double neighborDistance;
    
#pragma mark -
#pragma mark Set Methods
    
#pragma mark -
#pragma mark Get Methods
    
#pragma mark -
#pragma mark Inclusion Relationship Methods
    bool isParentOf(Object obj);
    bool isChildOf(Object obj);
    void mergeObject(Object obj);
    
private:
    
#pragma mark -
#pragma mark Compute Properties Methods
    void computeProperties();
    void computeColor();
    void computeContourArea(int gridSize);
    
    
};