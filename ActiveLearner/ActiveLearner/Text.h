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
    vector<int> originIndexes;
    
    int width = 0, height = 0;
    double aspectRatio = 0, gradient = 0, objectAreaRatio = 0;
    cv::Rect rect;
    cv::Point centroid;
    Scalar color;
    double averaveDistance = 0;
    
    int focusedIndex = -1;
    vector<double> features;
    
private:
    
    
public:
    Text();
    Text(string name, Object*& obj);
    
    void add(Object*& obj, double distance);
    void addDistance(double distance);
    bool areAllFocused();
    bool contains(Object*& obj);
    
private:
    void computeProperties();
    void computeColor();
    void computeAverageDistance();
    void computeGradient(Object*& obj);
    
};