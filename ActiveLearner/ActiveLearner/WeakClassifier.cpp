
#include "WeakClassifier.h"

WeakClassifier::WeakClassifier()
{
    
}

WeakClassifier::WeakClassifier(int index)
{
    featureIndex = index;
}

void WeakClassifier::train(vector<Sample> samples)
{
    int min_error = INFINITY;
    double min_t = 0;
    
    for (double t = 0; t < 1; t += 0.01)
    {
        int error = 0;
        threshold = t;
        
        for (int i = 0; i < samples.size(); i++)
        {
            if (test(samples[i]) != samples[i].label) error++;
        }
        
        if (error < min_error) {
            min_error = error;
            min_t = threshold;
        }
    }
    
    threshold = min_t;
}

int WeakClassifier::test(Sample &s)
{
    return s.features[featureIndex] >= threshold ? 1 : 0;
}

int WeakClassifier::test(Object *o)
{
    return o->features[featureIndex] >= threshold ? 1 : 0;
}

int WeakClassifier::test(Text *t)
{
    return t->features[featureIndex] >= threshold ? 1 : 0;
}