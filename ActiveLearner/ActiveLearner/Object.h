#pragma once

#include <iostream>

using namespace std;
using namespace cv;

class Object
{
public:
    Object();
    Object(const string ID, const string& filepath, const vector<cv::Point>& contour, const cv::Size size);
    ~Object();
    
#pragma mark -
#pragma Attribute Variables
public:
    string ID;
    string filename;
    cv::Size srcSize;
    
    int contourArea = 0, rectArea = 0, width = 0, height = 0;
    
    double rectRatio = 0, aspectRatio = 0, longLengthRatio = 0, areaRatio = 0;
    double longLength = 0, Gangle = 0, Fcorr = 0, Echar = 0, strokeWidth = 0;
    
    cv::Point origin, tp, lp, rp, bp, centroid = cv::Point(-1, -1);
    
    cv::Size size;
    
    cv::Rect rect;
    
    int mserIndex = -1, nearestIndex = -1;
    
    bool isPositive = true;

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
    vector<double> features;
    
    vector<Scalar> colors;
    Scalar color;
    
    bool grouped = false;
    vector<Object> neighbors;
    double neighborDistance = INFINITY;
    
    
#pragma mark -
#pragma mark Inclusion Relationship Methods
public:
    static int isLeftLarge(Object* obj1, Object* obj2) {
        return (obj1->rectArea > obj2->rectArea);
    }
    static int isLeftSmall(Object* obj1, Object* obj2) {
        return (obj1->rectArea < obj2->rectArea);
    }
    bool isParentOf(Object obj);
    bool isChildOf(Object obj);
    void mergeObject(Object obj);
    
#pragma mark -
#pragma mark Compute Properties Methods
private:
    void computeProperties();
    void computeContourArea(int gridSize);
    void computeTLRB();
    void uniqueContour();
    
public:
    void computeColor(vector<Scalar>& colors);

#pragma mark -
#pragma mark Operator
    
    bool operator==(Object& obj) {
        if (ID == obj.ID) return true;
        else return false;
    }
    bool operator!=(Object& obj) {
        if (ID != obj.ID) return true;
        else return false;
    }

};




