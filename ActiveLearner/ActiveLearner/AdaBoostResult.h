#pragma once

#include <iostream>
#include "WeakClassifier.h"
#include "StrongClassifier.h"

using namespace std;

class AdaBoostResult {
  
public:
    StrongClassifier sc;
    WeakClassifier wc;
    
public:
    AdaBoostResult(StrongClassifier& strong, WeakClassifier& selected);
    
};