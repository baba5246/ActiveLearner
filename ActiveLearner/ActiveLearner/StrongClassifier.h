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
    int test(Sample& s);
    int test(Object* o);
    int test(Text* t);
    
};