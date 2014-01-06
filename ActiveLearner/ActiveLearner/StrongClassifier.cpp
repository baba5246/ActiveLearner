
#include "StrongClassifier.h"

StrongClassifier::StrongClassifier()
{
    thresholdTrimming = 1.0;
}

int StrongClassifier::test(Sample& s)
{
    double threshold = 0;
    for (int c = 0; c < wcs.size(); c++) threshold += wcs[c].alpha;
    
    double discrimination = 0;
    for (int c = 0; c < wcs.size(); c++) discrimination += (wcs[c].alpha * wcs[c].Test(s));
    
    double adjustedThreshold = threshold * thresholdTrimming;
    
    return discrimination >= adjustedThreshold ? 1 : 0;
}


int StrongClassifier::test(Object* o)
{
    double threshold = 0;
    for (int c = 0; c < wcs.size(); c++) threshold += wcs[c].alpha;
    
    double discrimination = 0;
    for (int c = 0; c < wcs.size(); c++) discrimination += (wcs[c].alpha * wcs[c].Test(o));
    
    double adjustedThreshold = threshold * thresholdTrimming;
    
    return discrimination >= adjustedThreshold ? 1 : 0;
}

int StrongClassifier::test(Text* t)
{
    double threshold = 0;
    for (int c = 0; c < wcs.size(); c++) threshold += wcs[c].alpha;
    
    double discrimination = 0;
    for (int c = 0; c < wcs.size(); c++) discrimination += (wcs[c].alpha * wcs[c].Test(t));
    
    double adjustedThreshold = threshold * thresholdTrimming;
    
    return discrimination >= adjustedThreshold ? 1 : 0;
}