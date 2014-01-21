
#include "StrongClassifier.h"

StrongClassifier::StrongClassifier()
{
    thresholdTrimming = 0.5f;
}

int StrongClassifier::test(const Sample& s)
{
    double threshold = 0;
    for (int c = 0; c < wcs.size(); c++) threshold += wcs[c].alpha;
    threshold = threshold * 0.5f;
    
    double discrimination = 0;
    for (int c = 0; c < wcs.size(); c++) discrimination += (wcs[c].alpha * wcs[c].Test(s));
    
    double adjustedThreshold = threshold * thresholdTrimming;
    
    cout << "Disc_s( " << s.object.ID << " ): " << discrimination << endl;
    return discrimination >= adjustedThreshold ? 1 : 0;
}

int StrongClassifier::test(const Object*& o)
{
    double threshold = 0;
    for (int c = 0; c < wcs.size(); c++) threshold += wcs[c].alpha;
    threshold = threshold * 0.5f;
    
    double discrimination = 0;
    for (int c = 0; c < wcs.size(); c++) discrimination += (wcs[c].alpha * wcs[c].Test(o));
    
    double adjustedThreshold = threshold * thresholdTrimming;
    
    cout << "Disc_o( " << o->ID << " ): " << discrimination << endl;
    return discrimination >= adjustedThreshold ? 1 : 0;
}

int StrongClassifier::test(const Text*& t)
{
    double threshold = 0;
    for (int c = 0; c < wcs.size(); c++) threshold += wcs[c].alpha;
    threshold = threshold * 0.3f;
    
    double discrimination = 0;
    for (int c = 0; c < wcs.size(); c++) discrimination += (wcs[c].alpha * wcs[c].Test(t));
    
    double adjustedThreshold = threshold * thresholdTrimming;
    
    cout << "Disc_t( " << t->filename << " ): " << discrimination << endl;
    return discrimination >= adjustedThreshold ? 1 : 0;
}