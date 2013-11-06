
//#ifndef __ActiveLearner__Mycv__
//#define __ActiveLearner__Mycv__

#pragma once

#include <iostream>
#include "Features.h"
#include "Object.h"
#include "Text.h"
#include "Draw.h"

using namespace std;
using namespace cv;

class Mycv
{
public:
    // コンストラクタ
    Mycv(const string& filepath);
    
    // 
    vector<Object>  detectObjects();
    vector<Text>    detectTexts();
    
private:
    Mat src;
    
};

//#endif /* defined(__ActiveLearner__Mycv__) */