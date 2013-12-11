#pragma once

#include <iostream>
#include "Object.h"

using namespace std;
using namespace cv;

class Text
{
    
public:
    
    string filename;
    vector<Object*> objects;
    vector<double> distances;
    vector<double> gradients;
    
    int width = 0, height = 0;
    double aspectRatio = 0, gradient = 0;
    cv::Rect rect;
    cv::Point centroid;
    Scalar color;
    
    vector<double> features;
    
private:
    
    
public:
    Text();
    Text(string name, Object* obj);
    
    void add(Object* obj);
    bool areAllGrouped();
    
private:
    void computeProperties();
    void computeColor();
    void computeDistances();
    void computeGradient(Object* obj);
    
};