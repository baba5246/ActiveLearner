
#include "AdaBoost.h"


AdaBoost::AdaBoost(vector<Sample>& samples, vector<WeakClassifier>& classifiers)
{
    this->train = samples;
    this->wcs = classifiers;
    this->sc = *new StrongClassifier();
    
    weights = vector<double>(train.size(), 1);
    initializeWeight();
}

AdaBoostResult AdaBoost::next()
{
    vector<double> errors(wcs.size(), 0);
 
    // Normalize
    normalizeWeight();
    
    // Updates errors
    for (int e = 0; e < train.size(); e++)
    {
        Sample s = train[e];
        for (int c = 0; c < wcs.size(); c++)
        {
            if (wcs[c].test(s) != s.label) errors[c] += weights[e];
        }
    }
    
    // Choose lowest error classifiers
    int lIndex = 0;
    double lerror = errors[lIndex];
    for (int e = 0; e < LENGTH(errors); e++)
    {
        if (lerror > errors[e]) {
            lerror = errors[e];
            lIndex = e;
        }
    }
    
    // Compute b and update sample weights
    double beta = lerror / (1 - lerror);
    WeakClassifier lwc = wcs[lIndex];
    for (int e = 0; e < train.size(); e++)
    {
        double b = beta;
        Sample s = train[e];
        
        if (lwc.test(s) != s.label) b = 1;
        weights[e] = weights[e] * b;
    }
    
    double alpha = log(1 / beta) / 2;
    lwc.alpha = alpha;
    sc.wcs.push_back(lwc);
    
    return AdaBoostResult(sc, lwc);
    
}


void AdaBoost::initializeWeight()
{
    int posi = 0;
    int nega = 0;
    for (int i = 0; i < train.size(); i++)
    {
        if (train[i].label == 0) nega++;
        else posi++;
    }
    
    for (int i = 0; i < train.size(); i++)
    {
        if (train[i].label == 0) weights[i] = (double)1 / (2 * nega);
        else weights[i] = (double)1 / (2 * posi);
    }
}

void AdaBoost::normalizeWeight()
{
    double total = 0;
    for (int w = 0; w < train.size(); w++) total += weights[w];
    if (total == 0) return;
    for (int w = 0; w < train.size(); w++) weights[w] = weights[w] / total;
}