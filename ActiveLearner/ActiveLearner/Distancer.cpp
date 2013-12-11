
#include "Distancer.h"

#pragma mark -
#pragma mark Interface Methods

double Distancer::distanceBetweenObjects(const Object& obj1, const Object& obj2)
{
    double min = INFINITY, d = 0;
    cv::Point p1, p2;
    for (int i = 0; i < obj1.contourPixels.size(); i++)
    {
        d = distanceOfPoints(obj1.contourPixels[i], obj2.centroid);
        if (min > d) {
            min = d;
            p1 = obj1.contourPixels[i];
        }
    }
    min = INFINITY;
    for (int i = 0; i < obj2.contourPixels.size(); i++)
    {
        d = distanceOfPoints(obj2.contourPixels[i], p1);
        if (min > d) {
            min = d;
            p2 = obj2.contourPixels[i];
        }
    }
    
    return d;
}

double Distancer::distanceOfObjects(const Object& obj1, const Object& obj2)
{
    cv::Point diff1 = obj2.centroid - obj1.centroid;
    cv::Point diff2 = obj1.centroid - obj2.centroid;
    
    double a = (double)diff1.y / diff1.x;
    
    double t1 = atan2(-diff1.y, diff1.x);
    double t2 = atan2(-diff2.y, diff2.x);
    
    int pattern1 = patternOfRadian(t1);
    int pattern2 = patternOfRadian(t2);
    
    cv::Point rep1;
    findRepresentativePoint(rep1, obj1, a, pattern1);
    cv::Point rep2;
    findRepresentativePoint(rep2, obj2, a, pattern2);
    
    double d = distanceOfPoints(rep1, rep2);
    
    return d;
}


double Distancer::distanceOfCentroids(const cv::Point c1, const cv::Point c2)
{
    return distanceOfPoints(c1, c2);
}

#pragma mark -
#pragma mark Assistant Methods

int Distancer::patternOfRadian(const double radian)
{
    int n = 0;
    if (radian <= M_PI && radian > M_PI_2) n = 2;
    else if (radian <= M_PI_2 && radian > 0) n = 1;
    else if (radian <= 0 && radian > -M_PI_2) n = 4;
    else  n = 3;
    
    return n;
}

double Distancer::distanceOfPoints(const cv::Point& p1, const cv::Point& p2)
{
    cv::Point diff = p1 - p2;
    return sqrt(diff.x*diff.x+diff.y*diff.y);
}

void Distancer::pointApartFromCentroid(cv::Point& rep, const cv::Point& centroid, const cv::Point& p1, const cv::Point& p2, const double a)
{
    double b = centroid.y - a * centroid.x;
    cv::Point tmp_p1, tmp_p2;
    double tmp_a = 1 / a;
    double tmp_b1 = 0, tmp_b2 = 0;
    
    tmp_b1 = p1.y - tmp_a * p1.x;
    tmp_p1.x = - (b - tmp_b1) / (a - tmp_a);
    tmp_p1.y = tmp_a * tmp_p1.x + tmp_b1;
    
    tmp_b2 = p2.y - tmp_a * p2.x;
    tmp_p2.x = - (b - tmp_b2) / (a - tmp_a);
    tmp_p2.y = tmp_a * tmp_p2.x + tmp_b2;
    
    //    cout << "centroid:" << centroid << ", rep1:" << tmp_p1 << ", rep2:" << tmp_p2 << endl;
    
    if (distanceOfPoints(tmp_p1, centroid) > distanceOfPoints(tmp_p2, centroid)) rep = tmp_p1;
    else rep = tmp_p2;
}

void Distancer::findRepresentativePoint(cv::Point& rep, const Object& obj, const double a, const int pattern)
{
    switch (pattern) {
        case 1: // tp か rp を採用
            pointApartFromCentroid(rep, obj.centroid, obj.tp, obj.rp, a);
            break;
        case 2: // tp - lp を採用
            pointApartFromCentroid(rep, obj.centroid, obj.tp, obj.lp, a);
            break;
        case 3: // lp - bp を採用
            pointApartFromCentroid(rep, obj.centroid, obj.bp, obj.lp, a);
            break;
        case 4: // rp - bp を採用
            pointApartFromCentroid(rep, obj.centroid, obj.bp, obj.rp, a);
            break;
    }
}
