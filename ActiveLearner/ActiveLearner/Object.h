#pragma once

#include <iostream>

using namespace std;
using namespace cv;

class Object
{
public:
    
    Object(const string& filepath);
    string filename;
    
    CvPoint origin;
    CvSize size;
    CvPoint centroid;
    double aspectRatio;
    CvRect rect;
    int64 rectArea;
    CvScalar color;
    
    double rectRatio;
    int nearestIndex;

    vector<int> parents;
    vector<int> children;
    
    vector<CvPoint> contourPixels;
    vector<CvPoint> corrPairPixels;
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
#pragma mark Inclusion Relationship Methods
    bool isParentOf(Object obj);
    bool isChildOf(Object obj);
    void mergeObject(Object obj);
    
private:
    
#pragma mark -
#pragma mark Compute Properties Methods
    void computeProperties();
    void computeCentroid();
    void computeContourArea();
    void computeColor();
    
    
};