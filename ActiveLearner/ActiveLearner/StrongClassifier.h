#pragma once

#include <iostream>
#include "WeakClassifier.h"
#include "Sample.h"

using namespace std;

class StrongClassifier {
    
public:
    vector<WeakClassifier> wcs;
    double thresholdTrimming = 0;
    
public:
    StrongClassifier();
    int test(const Sample& s);
    int test(const Object*& o);
    int test(const Text*& t);
    
};