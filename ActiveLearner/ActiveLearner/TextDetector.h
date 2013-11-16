#pragma once

#include <iostream>

#include "Object.h"
#include "Text.h"
#include "Draw.h"


using namespace std;
using namespace cv;


class TextDetector {

private:
    
    cv::Mat srcImage;
    long recursive_count = 0;
    
public:
    
    TextDetector();
    TextDetector(const Mat& src);
    ~TextDetector();
    
    // Grouping Methods
    void detect(vector<Object*>& objects, vector<Text>& texts);
    
private:
    
    void groupingObjects(Text& text, Object& obj, vector<double>& distance, vector<Object*>& objects);
    void findNeighbors(vector<Object*>& neighbors, Object& obj, vector<double>& distance, vector<Object*>& objects);
    void adding(Text& text, vector<Object*>& neighbors);
    bool isSimilar(Scalar c1, Scalar c2);
    
    double distanceWithNearestNeighbor(Object& obj, vector<Object*>& objects);
    double distanceOfObjects(const Object& obj1, const Object& obj2);
    double distanceOfPoints(const cv::Point& p1, const cv::Point& p2);
    int patternOfRadian(const double radian);
    void pointApartFromCentroid(cv::Point& rep, const cv::Point& centroid, const cv::Point& p1, const cv::Point& p2, const double a);
    void findRepresentativePoint(cv::Point& rep, const Object& obj, const double a, const int pattern);
    
};
