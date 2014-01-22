
#include "SelfTrainer.h"


AdaBoost SelfTrainer::selfTraining(vector<Sample>& labels, vector<Sample>& unlabels, int iteration)
{
    vector<Sample> tempSamples;
    tempSamples.insert(tempSamples.end(), labels.begin(), labels.end());
    
    AdaBoost adaboost;
    
    // Training loop
    for (int i = 0; i < iteration; i++) {
        
        if (i > 0) {
            // Create Virtual Representatives
            vector<Sample> vrs = creatVirtualRepresentatives(adaboost, tempSamples, unlabels);
            tempSamples.insert(tempSamples.end(), vrs.begin(), vrs.end());
        }
        
        // WeakClassifierを準備
        vector<WeakClassifier> classifiers;
        for (int i = 0; i < tempSamples[0].features.size(); i++) {
            WeakClassifier wc(i);
            classifiers.push_back(wc);
        }
        
        // AdaBoost learning
        AdaBoost boost(tempSamples, classifiers);
        for (int t = 0; t < tempSamples[0].features.size(); t++) {
            AdaBoostResult result = boost.next();
            WeakClassifier selected = result.wc;
            cout << "t:" << t << ", wc index:" << selected.featureIndex
                 << ", alpha:" << selected.alpha << endl;
        }
        
        // AdaBoostの更新
        adaboost = AdaBoost(boost);
    }
    
    return adaboost;
}

vector<Sample> SelfTrainer::creatVirtualRepresentatives(AdaBoost& adaboost, vector<Sample>& labels, vector<Sample>& unlabels)
{
    vector<Sample> VRs;
    
    srand((unsigned int)time(NULL));
    int random = rand() % unlabels.size();
    
    for (int i = 0; i < random; i++) {
        int rIndex = rand() % unlabels.size();
        Sample s(unlabels[rIndex]);
        s.label = adaboost.sc.test(s);
        VRs.push_back(s);
    }
    
    return VRs;
}