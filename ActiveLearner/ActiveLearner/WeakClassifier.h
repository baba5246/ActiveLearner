#pragma once

#include <iostream>
#include "Sample.h"

using namespace std;

class WeakClassifier {
    
public:
    int featureIndex = 0;
    double alpha = 0;
    double threshold = 0;
    
public:
    WeakClassifier();
    WeakClassifier(int index);
    
    void train(vector<Sample> samples);
    
    int test(Sample& s);
    int test(Object* o);
    int test(Text* t);
};