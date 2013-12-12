#pragma once

#include <iostream>

#include "Object.h"
#include "Text.h"
#include "Draw.h"
#include "Distancer.h"


#define LOW_GRADIENT_THRESHOLD  M_PI_6
#define HIGH_GRADIENT_THRESHOLD  M_PI_5_6

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
    
    vector<Object*> findInitNeighbors(Object* init, vector<Object*> objects);
    vector<double> distanceObjects(vector<Object*> objects);
    void groupingObjects(Text*& text, vector<Object*> objects);
    void addNeighbors(Text* text, vector<Object*> objects);
    
    double computeGradient(Object obj1, Object obj2);
    double pointSize(cv::Point p);
};
