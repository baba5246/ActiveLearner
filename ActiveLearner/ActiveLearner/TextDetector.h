#pragma once

#include <iostream>

#include "Object.h"
#include "Text.h"
#include "Draw.h"
#include "Distancer.h"


#define LOW_GRADIENT_THRESHOLD      M_PI_6
#define HIGH_GRADIENT_THRESHOLD     M_PI_5_6
#define STROKE_WIDTH_RATIO          3.0
#define RECT_MARGE_THRESHOLD1       0.3
#define RECT_HABA_MARGE_THRESHOLD   0.8
#define RECT_HABA_ALMOST_CONTAIN    0.90
#define ALMOST_CONTAIN_THRESHOLD    0.95
#define LAB_COLOR_SIMILARITY        20
#define SAME_LINK_COUNT             3

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
    void mergeFilteredTexts(vector<Text*>& dst_texts, vector<Text*>& src_texts);
    void mergeContainedTexts(vector<Text*>& dst_texts, vector<Text*>& src_texts);
    
private:
    
    void detectTexts(vector<Text*>& texts, vector<Object*>& objects);
    vector<Object*> findInitNeighbors(Object*& init, vector<Object*>& objects);
    void groupingObjects(Text*& text, vector<Object*>& objects);
    void addNeighbors(Text*& text, vector<Object*>& objects);
    void textFiltering(vector<Text*>& dst_texts, vector<Text*>& src_texts);
    void setFeatures(vector<Text*>& texts, vector<Object*>& objects);
};
