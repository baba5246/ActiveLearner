
#include "SWTObject.h"

SWTObject::SWTObject()
{
    
}

SWTObject::SWTObject(vector<Point> region, const Mat& swts)
{
    this->swt = swts;
    this->region = region;
    computeProperties();
}

void SWTObject::computeProperties()
{
    int t = MAXFLOAT, b = 0, l = MAXFLOAT, r = 0;
    long x = 0, y = 0;
    for (int i = 0; i < region.size(); i++) {
        if (t > region[i].y) t = region[i].y;
        if (b < region[i].y) b = region[i].y;
        if (l > region[i].x) l = region[i].x;
        if (r < region[i].x) r = region[i].x;
        x += region[i].x;
        y += region[i].y;
    }
    // 幅と高さの計算
    width = r-l;
    height = b-t;
    // 最小矩形領域の計算
    rect = Rect(l, t, width, height);
    // 重心の計算
    if (region.size()>0) {
        x /= region.size();
        y /= region.size();
    }
    centroid = Point((int)x, (int)y);
    // アスペクト比の計算
    aspectRatio = MAX((double)rect.width, (double)rect.height) / MIN((double)rect.width, (double)rect.height);
    // Stroke Width の平均と分散の計算
    computeMeanAndVariance();
    // 回転矩形の計算
    rotatedRect = minAreaRect(region);
    // 面積比の計算
    areaRatio = (double)region.size() / rotatedRect.size.area();
}

void SWTObject::computeMeanAndVariance()
{
    double m = 0, var = 0;
    for (int i = 0; i < region.size(); i++) {
        m += swt.at<double>(region[i].y, region[i].x);
    }
    if (region.size() > 0) m /= region.size();
    
    for (int i = 0; i < region.size(); i++) {
        var += pow(m - swt.at<double>(region[i].y, region[i].x), 2);
    }
    if (region.size() > 0) var /= region.size();
    
    mean = m;
    variance = var;
}