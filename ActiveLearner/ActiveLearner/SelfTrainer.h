#pragma once

#include <iostream>
#include "AdaBoost.h"
#include "Sample.h"

class SelfTrainer {
    
    
public:
    
    static AdaBoost selfTraining(vector<Sample>& labels, vector<Sample>& unlabels, int iteration);
    
    
private:
    
    static vector<Sample> creatVirtualRepresentatives(AdaBoost& adaboost, vector<Sample>& labels, vector<Sample>& unlabels);
    
    
};