
#include "WeakClassifier.h"

WeakClassifier::WeakClassifier()
{
    
}

WeakClassifier::WeakClassifier(int index)
{
    featureIndex = index;
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