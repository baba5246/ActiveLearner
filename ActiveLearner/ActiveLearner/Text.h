#pragma once

#include <iostream>
#include "Object.h"

using namespace std;
using namespace cv;

class Text
{
    
public:
    
    Mat srcImage;
    string filepath, filename;
    vector<Object*> objects;
    vector<double> distances;
    vector<double> gradients;
    vector<int> originIndexes;
    
    int srcW = 0, srcH;
    int width = 0, height = 0;
    double aveEchar = 0, aveFcorr = 0, aveGangle = 0, aveCR = 0, aveDist = 0, aveSW = 0;
    double varSW = 0, varColorR = 0, varColorG = 0, varColorB = 0, varAngle = 0, varDist = 0, varLength = 0;
    double rectRatio = 0, aspectRatio = 0, longLengthRatio = 0, objAreaRatio = 0, objSizeRatio = 0;
    cv::Rect rect;
    cv::Point centroid;
    Scalar color;
    
    int focusedIndex = -1;
    vector<double> features;
    
private:
    
    
public:
    Text();
    Text(string path, Object*& obj);
    
    void add(Object*& obj, double distance);
    void add(Text*& text);
    void addDistance(double distance);
    bool areAllFocused();
    bool contains(Object*& obj);
    void computeProperties();
    
private:
    void computeColor();
    void computeAverageDistance();
    void computeGradients();
    void computeStrokeWidth();
    
    void computeAverageFeatures();
    void computeVariantFeatures();
    void computeRatioFeatures();
    double computeObjectArea();
    
};