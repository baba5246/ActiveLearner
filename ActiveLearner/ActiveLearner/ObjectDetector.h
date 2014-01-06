#pragma once

#include <iostream>
#include <sstream>
#include "Mycv.h"
#include "Object.h"
#include "MSERegion.h"
#include "Draw.h"
#include "TextDetector.h"

using namespace std;
using namespace cv;

class ObjectDetector {
    
public:
    
private:
    
    cv::Mat srcImage;
    IplImage srcIplImage;
    string filename;
    long id_count = 0;
    
public:
    
    // Contructors
    ObjectDetector();
    ObjectDetector(const string& filepath);
    ~ObjectDetector();
    
    // Interface Methods
    void detect(vector<Object*>& objects);
    
private:
    
    // Contour Interpolating Methods
    void createObjects(const vector<vector<cv::Point> >& contours, vector<Object*>& objects);
    void mergeApartContours(vector<Object*>& objects, vector<MSERegion>& msers);
    void mergeIncludedObjects(vector<Object*>& objects);
    
    // Compute Feature Methods
    void gradientOfObjects(vector<Object*>& objects, const Mat_<double>& gradients);
    bool isPositiveDirection(Object* object);
    Scalar getColor(int x, int y);
    void findCorrPairs(vector<Object*>& objects, const Mat& gradients);
    void gradientOfCorrPairs(vector<Object*>& objects, const Mat_<double>& gradients);
    void computeEchar(vector<Object*>& objects);
    void computeStrokeWidth(vector<Object*>& objects);
    
    void setFeatures(vector<Object*>& objects);
    
    
    vector<string> split(const string& str, char delim);
    int** createImageTable(const vector<Object*>& objects);
    cv::Rect* intersect(const cv::Rect& rect1, const cv::Rect& rect2);
    
    void SWTComponent(const Mat& src, vector<Object>& component);
};

