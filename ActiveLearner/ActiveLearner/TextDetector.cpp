
#include "TextDetector.h"

#pragma mark -
#pragma mark Constructors

TextDetector::TextDetector()
{
    
}

TextDetector::TextDetector(const Mat& src)
{
    srcImage = src;
}

TextDetector::~TextDetector()
{
    
}


#pragma mark -
#pragma mark Interface Methods

void TextDetector::detect(vector<Object*>& objects, vector<Text>& texts)
{
    // Sort
    sort(objects.begin(), objects.end(), Object::isLeftSmall);
    
    // Grouping アルゴリズム
    long length = objects.size();
    for (int i = 0; i < length; i++)
    {
        Object *obj = objects[i];
        if (obj->grouped) continue;
        
        Text text = *new Text(*obj);
        obj->grouped = true;
        
        vector<double> distance;
        double initd = 2 * distanceWithNearestNeighbor(*obj, objects);
        distance.push_back(MIN(initd, obj->longLength));
        
        recursive_count = 0;
        groupingObjects(text, *obj, distance, objects);
        
        if (text.aspectRatio >= 0.8 && text.aspectRatio <= 1.2)
        {
            distance.clear();
            distance.push_back(text.width);
            groupingObjects(text, *obj, distance, objects);
        }
        
        texts.push_back(text);
    }
}


#pragma mark -
#pragma mark Grouping Objects Methods

void TextDetector::groupingObjects(Text& text, Object& obj, vector<double>& distance, vector<Object*>& objects)
{
    recursive_count++;
    cout<< "Recursive Count:" << recursive_count << endl;
    
    vector<Object*> neighbors;
    findNeighbors(neighbors, obj, distance, objects);
    adding(text, neighbors);
    
    for (int i = 0; i < neighbors.size(); i++) {
        groupingObjects(text, *neighbors[i], distance, objects);
    }
}

double TextDetector::distanceWithNearestNeighbor(Object& obj, vector<Object*>& objects)
{
    double min = INFINITY;
    for (int i = 0; i < objects.size(); i++)
    {
        if (obj.ID == objects[i]->ID) continue;
        double d = distanceOfObjects(obj, *objects[i]);
        if (min > d) min = d;
    }
    return min;
}

void TextDetector::findNeighbors(vector<Object*>& neighbors, Object& obj, vector<double>& distance, vector<Object*>& objects)
{
    double temp_dist = 0, average = 0;
    
    for (int i = 0; i < objects.size(); i++)
    {
        average = 0;
        if (obj != *objects[i] && !(objects[i]->grouped))
        {
            average = accumulate(distance.begin(), distance.end(), 0);
            average /= (double)distance.size();
            temp_dist = distanceOfObjects(obj, *objects[i]);
            
            if (temp_dist < average) {
                if (obj.neighborDistance > temp_dist) {
                    obj.neighborDistance = temp_dist;
                    if (obj.neighbors.size() > 0) obj.neighbors.pop_back();
                    obj.neighbors.push_back(*objects[i]);
                }
                neighbors.push_back(objects[i]);
                distance.push_back(temp_dist);
            }
        }
    }
}

double TextDetector::distanceOfObjects(const Object& obj1, const Object& obj2)
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
    cout << "Obj1:" << obj1.centroid << ", Rep1:" << rep1 << ", Obj2:" << obj2.centroid << ", Rep2:"
    << rep2 << ", D:" << d << endl;
    
    return d;
}

int TextDetector::patternOfRadian(const double radian)
{
    int n = 0;
    if (radian <= M_PI && radian > M_PI_2) n = 2;
    else if (radian <= M_PI_2 && radian > 0) n = 1;
    else if (radian <= 0 && radian > -M_PI_2) n = 4;
    else  n = 3;
    
    return n;
}

double TextDetector::distanceOfPoints(const cv::Point& p1, const cv::Point& p2)
{
    cv::Point diff = p1 - p2;
    return sqrt(diff.x*diff.x+diff.y*diff.y);
}

void TextDetector::pointApartFromCentroid(cv::Point& rep, const cv::Point& centroid, const cv::Point& p1, const cv::Point& p2, const double a)
{
    double b = centroid.y - a * centroid.x;
    cv::Point tmp_p1, tmp_p2;
    double tmp_a = 1 / a;
    double tmp_b1 = 0, tmp_b2 = 0;
    
    tmp_b1 = p1.y - tmp_a * p1.x;
    tmp_p1.x = - (b - tmp_b1) / (a - tmp_a);
    tmp_p1.y = a * tmp_p1.x + b;
    
    tmp_b2 = p2.y - tmp_a * p2.x;
    tmp_p2.x = - (b - tmp_b2) / (a - tmp_a);
    tmp_p2.y = a * tmp_p2.x + b;
    
    cout << "centroid:" << centroid << ", rep1:" << tmp_p1 << ", rep2:" << tmp_p2 << endl;
    if (distanceOfPoints(tmp_p1, centroid) > distanceOfPoints(tmp_p2, centroid)) rep = tmp_p1;
    else rep = tmp_p2;
}

void TextDetector::findRepresentativePoint(cv::Point& rep, const Object& obj, const double a, const int pattern)
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

void TextDetector::adding(Text& text, vector<Object*>& neighbors)
{
    vector<int> rmIndex;
    for (int i = 0; i < neighbors.size(); i++)
    {
        Object* n = neighbors[i];
        if (n->grouped) continue;
        
        if (n->isPositive == text.objects[0].isPositive) {
            text.add(*n);
            n->grouped = true;
        } else {
            rmIndex.push_back(i);
        }
    }
    for (int i = (int)rmIndex.size()-1; i > -1; i--) {
        neighbors.erase(neighbors.begin()+rmIndex[i]);
    }
}

bool TextDetector::isSimilar(Scalar c1, Scalar c2)
{
    Scalar diff = c1 - c2;
    double dist = sqrt(diff[0]*diff[0]+diff[1]*diff[1]+diff[2]*diff[2]);
    if (dist <= 50) return true;
    
    return false;
}
