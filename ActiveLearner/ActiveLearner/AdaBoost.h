#pragma once

#include <iostream>
#include "Sample.h"
#include "WeakClassifier.h"
#include "StrongClassifier.h"
#include "AdaBoostResult.h"

using namespace std;

class AdaBoost{
    
    // インスタンス変数
    
public:
    // メンバ関数
    AdaBoost(vector<Sample>& samples, vector<WeakClassifier>& classifiers);
    AdaBoostResult next();
    
private:
    void initializeWeight();
    void normalizeWeight();
    
public:
    double *weights;
    StrongClassifier sc;
    
private:
    vector<Sample> train;
    vector<WeakClassifier> wcs;
    
};