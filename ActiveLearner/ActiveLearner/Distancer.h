#pragma once

#include <iostream>

#include "Object.h"
#include "Text.h"

class Distancer {
    
    
public:
    
    static double distanceBetweenObjects(const Object& obj1, const Object& obj2);
    static double distanceOfObjects(const Object& obj1, const Object& obj2);
    static double distanceOfCentroids(const cv::Point c1, const cv::Point c2);
    
private:
    
    static int patternOfRadian(const double radian);
    static double distanceOfPoints(const cv::Point& p1, const cv::Point& p2);
    static void pointApartFromCentroid(cv::Point& rep, const cv::Point& centroid, const cv::Point& p1, const cv::Point& p2, const double a);
    static void findRepresentativePoint(cv::Point& rep, const Object& obj, const double a, const int pattern);
    
};