
#include "MSERegion.h"

MSERegion::MSERegion(vector<cv::Point> mser)
{
    mseregion = mser;
    rect = boundingRect(mseregion);
}

MSERegion::~MSERegion()
{
    
}