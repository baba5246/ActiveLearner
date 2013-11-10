#pragma once

#include <iostream>

using namespace std;
using namespace cv;

class MSERegion
{
public:
    MSERegion(vector<cv::Point> mser);
    ~MSERegion();
    
    vector<cv::Point> mseregion;
    cv::Rect rect;
    vector<int> objIndexes;
    
private:
    
};