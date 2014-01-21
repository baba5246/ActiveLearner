#pragma once

#include <iostream>
#include "Sample.h"

using namespace std;

class WeakClassifier {
    
public:
    int featureIndex = 0;
    double alpha = 0;
    double threshold = 0;
    int type = 1;
    
public:
    WeakClassifier();
    WeakClassifier(int index);
    
    void train(const vector<Sample>& samples);
    
    int Test(const Sample &s);
    int Test(const Object*& o);
    int Test(const Text*& t);
    
private:
    int test(const Sample& s);
    int test(const Object*& o);
    int test(const Text*& t);
    int inverse(const int label);
};