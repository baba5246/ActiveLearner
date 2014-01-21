#pragma once

#include <iostream>
#include "Object.h"
#include "Text.h"

using namespace std;

class Sample {
    
public:
    Object *object;
    Text *text;
    vector<double> features;
    int label;
    string filename;
    
public:
    Sample(Object*& obj);
    Sample(Text*& t);
    string toString();
};