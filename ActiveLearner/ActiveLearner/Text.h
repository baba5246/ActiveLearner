#pragma once

#include <iostream>
#include "Object.h"

using namespace std;
using namespace cv;


#define HIGH_GRADIENT_THRESHOLD     M_PI_5_6


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
    double rectArea = 0, objectArea = 0, textArea = 0;
    double objectCount = 0, falseCount = 0;
    cv::Rect rect;
    cv::RotatedRect rotatedRect;
    cv::Point centroid;
    Scalar color, labcolor;
    
    // Average Features
    double aveEchar = 0, aveFcorr = 0, aveGangle = 0, aveColorSim = 0, aveLabSim = 0;
    double aveCR = 0, aveSW = 0, aveDist = 0, aveAngle = 0, aveAspect = 0, aveCircleR = 0;
    
    // Variation Features
    double varSW = 0, varWidth = 0, varHeight = 0, varAngle = 0, varDist = 0;
    
    // Ratio Features
    double rectRatio = 0, aspectRatio = 0, longLengthRatio = 0, objAreaRatio = 0, objSizeRatio = 0, trueObjectRatio = 0;
    
    int focusedIndex = -1;
    vector<double> features;
    
private:
    
    
public:
    Text();
    Text(string path, Object*& obj);
    
    void add(Object*& obj, double distance);
    void add(Text*& text);
    void reLinkOriginIndexes();
    void addDistance(double distance);
    bool areAllFocused();
    bool contains(Object*& obj);
    void computeProperties();
    void countFalseObjects(vector<Object*>& objs);
    
    
    static int isLeftLarge(Text* t1, Text* t2) {
        return (t1->rectArea > t2->rectArea);
    }
    static int isLeftSmall(Text* t1, Text* t2) {
        return (t1->rectArea < t2->rectArea);
    }
    
private:
    void computeColor();
    void computeAverageDistance();
    void computeGradients();
    void computeStrokeWidth();
    
    void computeAverageFeatures();
    void computeVariantFeatures();
    void computeRatioFeatures();
    double computeObjectArea();
    double computeTextArea();
    
};