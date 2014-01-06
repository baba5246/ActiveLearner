
#include "WeakClassifier.h"

WeakClassifier::WeakClassifier()
{
    
}

WeakClassifier::WeakClassifier(int index)
{
    this->featureIndex = index;
}

void WeakClassifier::train(vector<Sample> samples)
{
    int min_error = INFINITY;
    double min_t = 0;
    
    for (double t = 0; t < 1; t += 0.01)
    {
        int error = 0, error1 = 0, error2 = 0;
        threshold = t;
        
        for (int i = 0; i < samples.size(); i++)
        {
            if (test(samples[i]) != samples[i].label) error1++;
            if (inverse(test(samples[i])) != samples[i].label) error2++;
        }
        
        error = MIN(error1, error2);
        
        if (error < min_error) {
            if (error == error1) type = 1;
            else if (error == error2) type = -1;
            
            min_error = error;
            min_t = threshold;
        }
    }
    
    threshold = min_t;
}

int WeakClassifier::test(Sample &s)
{
    return s.features[this->featureIndex] >= threshold ? 1 : 0;
}

int WeakClassifier::test(Object *o)
{
    return o->features[featureIndex] >= threshold ? 1 : 0;
}

int WeakClassifier::test(Text *t)
{
    return t->features[featureIndex] >= threshold ? 1 : 0;
}

int WeakClassifier::inverse(int label)
{
    return label == 0 ? 1 : 0;
}

int WeakClassifier::Test(Sample &s)
{
    if (type > 0) return test(s);
    else return inverse(test(s));
}

int WeakClassifier::Test(Object *o)
{
    if (type > 0) return test(o);
    else return inverse(test(o));
}

int WeakClassifier::Test(Text *t)
{
    if (type > 0) return test(t);
    else return inverse(test(t));
}