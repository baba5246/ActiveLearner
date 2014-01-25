#pragma once

#include <iostream>

using namespace std;
using namespace cv;

class Object
{
public:
    Object();
    Object(const string ID, const string& path, const vector<cv::Point>& contour, const cv::Size size);
    ~Object();
    
#pragma mark -
#pragma Attribute Variables
public:
    string ID;
    string filepath, filename;
    cv::Size srcSize;
    
    int contourArea = 0, rectArea = 0, width = 0, height = 0;
    int mInnerArea = 0, pInnerArea = 0,  innerArea = 0;
    
    double rectRatio = 0, aspectRatio = 0, longLengthRatio = 0, areaRatio = 0;
    double longLength = 0, Gangle = 0, Fcorr = 0, Echar = 0, CR = 0;
    double strokeWidth = 0, varStrokeWidth = 0;
    
    cv::Point origin, centroid;
    cv::Point tp, lp, rp, bp;
    cv::Size size;
    cv::Rect rect;
    
    cv::Point2f center;
    float r = 0;
    
    int mserIndex = -1, nearestIndex = -1;
    
    int gradientType = 0;

    vector<int> parents;
    vector<int> children;
    
    Mat_<int> innerAreaMap;
    
    vector<cv::Point> surroundings;
    vector<cv::Point> contourPixels;
    vector<cv::Point> corrPairPixels;
    vector<cv::Point> mCorrPairPixels, pCorrPairPixels;
    
    vector<double> thetas;
    vector<double> mCorrThetas;
    vector<double> pCorrThetas;
    vector<double> corrThetas;
    vector<double> surrThetas;
    vector<double> features;
    
    vector<Scalar> colors;
    Scalar color;
    Scalar labcolor;
    
    bool didFocused = false;
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
    static int leftToRight(Object* obj1, Object* obj2) {
        return (obj1->lp.x < obj2->lp.x);
    }
    bool isParentOf(Object obj);
    bool isChildOf(Object obj);
    void mergeObject(Object obj);
    
#pragma mark -
#pragma mark Compute Properties Methods
private:
    void computeProperties();
    void computeTLRB();
    void computeContourArea(int gridSize);
    void uniqueContour();
    
    void computeStrokeWidth();
    void computeFromInnerAreaMap(Mat& src);
    void computeEchar();
    void computeInnerArea();
    
public:
    void computeFeatures(Mat& srcImage);

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




