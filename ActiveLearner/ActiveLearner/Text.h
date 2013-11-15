#pragma once

#include <iostream>
#include "Object.h"

using namespace std;
using namespace cv;

class Text
{
    
public:
    vector<Object> objects;
    
    int width = 0, height = 0;
    double aspectRatio = 0;
    cv::Rect rect;
    cv::Point centroid;
    Scalar color;
    
private:
    
    
public:
    Text();
    Text(Object& obj);
    
    void add(Object& obj);
    bool areAllGrouped();
    
private:
    void computeProperties();
    void computeColor();
    
};