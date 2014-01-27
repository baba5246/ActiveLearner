
#include "WeakClassifier.h"

WeakClassifier::WeakClassifier()
{
    
}

WeakClassifier::WeakClassifier(int index)
{
    this->featureIndex = index;
}

void WeakClassifier::train(const vector<Sample>& samples)
{
    int min_error = INFINITY;
    double min_t = 0;
    double max_elem = 0, itr = 0, max = 0;
    
    if (samples[0].hasObject) {
        
        max = 1.0;
        itr = 0.01;
        
    } else {
        
        for (int i = 0; i < samples.size(); i++) {
            if (max_elem < samples[i].features[featureIndex])
                max_elem = samples[i].features[featureIndex];
        }
        for (double i = -3; i < 11; i++) {
            if (max_elem <= pow(10, i)) {
                max = pow(10, i);
                break;
            }
        }
        
        itr = max * 0.001;
    }
    
    for (double t = 0; t < max; t += itr)
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

int WeakClassifier::test(const Sample &s)
{
    return s.features[this->featureIndex] >= threshold ? 1 : 0;
}

int WeakClassifier::test(const Object*& o)
{
    return o->features[featureIndex] >= threshold ? 1 : 0;
}

int WeakClassifier::test(const Text*& t)
{
    return t->features[featureIndex] >= threshold ? 1 : 0;
}

int WeakClassifier::inverse(const int label)
{
    return label == 0 ? 1 : 0;
}

int WeakClassifier::Test(const Sample &s)
{
    if (type > 0) return test(s);
    else return inverse(test(s));
}

int WeakClassifier::Test(const Object*& o)
{
    if (type > 0) return test(o);
    else return inverse(test(o));
}

int WeakClassifier::Test(const Text*& t)
{
    if (type > 0) return test(t);
    else return inverse(test(t));
}