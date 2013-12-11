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
    void detect(vector<Object*>& objects, vector<Text*>& texts);
    
private:
    
    vector<Object*> findInitNeighbors(Object* init);
    vector<double> distanceObjects(vector<Object*> objects);
    void groupingObjects(Text*& text, vector<Object*> objects);
    
    
    void groupingObjects(Text& text, Object& obj, vector<double>& distance, vector<Object*>& objects);
    void findNeighbors(vector<Object*>& neighbors, Object& obj, vector<double>& distance, vector<Object*>& objects);
    void adding(Text& text, vector<Object*>& neighbors, vector<double>& distance);
    bool isSimilar(Scalar c1, Scalar c2);
    
    double distanceWithNearestNeighbor(Object& base, Object*& nearest, vector<Object*>& objects);
    double distanceBetweenObjects(const Object& obj1, const Object& obj2);
    double distanceOfObjects(const Object& obj1, const Object& obj2);
    double distanceOfPoints(const cv::Point& p1, const cv::Point& p2);
    int patternOfRadian(const double radian);
    void pointApartFromCentroid(cv::Point& rep, const cv::Point& centroid, const cv::Point& p1, const cv::Point& p2, const double a);
    void findRepresentativePoint(cv::Point& rep, const Object& obj, const double a, const int pattern);
    
    bool isLeftContained(cv::Rect s, cv::Rect l);
    bool isSimilarGradient(Text& text, Object& obj);
    cv::Rect* intersect(const cv::Rect& rect1, const cv::Rect& rect2);
};
