#pragma once

#include <iostream>
#include "Object.h"

using namespace std;

class Sample {
    
public:
    Object object;
    vector<double> features;
    int label;
    string filename;
    
public:
    Sample(const Object& obj);
    string toString();
};