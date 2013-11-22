
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
    sort(objects.begin(), objects.end(), Object::leftToRight);
    
    // Grouping アルゴリズム
    for (int i = 0; i < objects.size(); i++)
    {
        if (objects[i]->Echar < 0.70f) continue;
        
        Object *obj = objects[i];
        if (obj->grouped) continue;
        
        Text text = *new Text(*obj);
        obj->grouped = true;
        
        vector<double> distance;
        
        recursive_count = 0;
        groupingObjects(text, *obj, distance, objects);
        
//        if (text.aspectRatio >= 0.8 && text.aspectRatio <= 1.2)
//        {
//            distance.clear();
//            distance.push_back(MIN(text.width, text.height));
//            groupingObjects(text, *obj, distance, objects);
//        }
        
        texts.push_back(text);
        
        //Draw::drawTexts(srcImage, texts);
    }
}


#pragma mark -
#pragma mark Grouping Objects Methods

void TextDetector::groupingObjects(Text& text, Object& obj, vector<double>& distance, vector<Object*>& objects)
{
    vector<Object*> neighbors;
    findNeighbors(neighbors, obj, distance, objects);
    adding(text, neighbors, distance);
    
    for (int i = 0; i < neighbors.size(); i++) {
        groupingObjects(text, *neighbors[i], distance, objects);
    }
}

double TextDetector::distanceWithNearestNeighbor(Object& base, Object*& nearest, vector<Object*>& objects)
{
    double min = INFINITY;
    for (int i = 0; i < objects.size(); i++)
    {
        if (base.ID == objects[i]->ID) continue;
        double d = distanceBetweenObjects(base, *objects[i]);
        if (min > d) {
            min = d;
            nearest = objects[i];
        }
    }
    return min;
}

void TextDetector::findNeighbors(vector<Object*>& neighbors, Object& obj, vector<double>& distance, vector<Object*>& objects)
{
    double temp_dist = 0, average = 0, sl = 0;
    
    for (int i = 0; i < objects.size(); i++)
    {
        if (objects[i]->Echar < 0.70f) continue;
        
        average = 0;
        if (obj != *objects[i] && !(objects[i]->grouped))
        {
            for (int i = 0; i < distance.size(); i++) average += distance[i];
            if (distance.size()>0) {
                average /= (double)distance.size();
                average = (average + obj.longLength) / 2;
            } else {
                average = obj.longLength;
            }
            
            temp_dist = distanceBetweenObjects(obj, *objects[i]);
            sl = (obj.rect.area() < objects[i]->rect.area()) ? obj.longLength : objects[i]->longLength;
            
            if (temp_dist < average*1.5f && temp_dist < sl) {
                neighbors.push_back(objects[i]);
            }
        }
    }
}

void TextDetector::adding(Text& text, vector<Object*>& neighbors, vector<double>& distance)
{
    vector<int> rmIndex;
    for (int i = 0; i < neighbors.size(); i++)
    {
        Object* n = neighbors[i];
        if (n->grouped) continue;
        
        if (n->isPositive == text.objects[0].isPositive &&
            !isLeftContained(n->rect, text.rect) &&
            isSimilarGradient(text, *n)) //&& isSimilarStrokeWidth(text, *n))
        {
            text.add(*n);
            distance.push_back(distanceBetweenObjects(text.objects[text.objects.size()-1], *n));
            n->grouped = true;
        }
        else
        {
            rmIndex.push_back(i);
        }
    }
    for (int i = (int)rmIndex.size()-1; i > -1; i--) {
        neighbors.erase(neighbors.begin()+rmIndex[i]);
    }
}

double TextDetector::distanceBetweenObjects(const Object& obj1, const Object& obj2)
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
    
    cout << "Obj1:{c:" << obj1.centroid << ", (" << obj1.width << ", " << obj1.height << ")}" << ", Rep1:" << rep1 << endl;
    cout << "Obj2:{c:" << obj2.centroid << ", (" << obj2.width << ", " << obj2.height << ")}" << ", Rep2:" << rep2 << endl;
    cout << "Distance:" << d << endl;
    
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
    tmp_p1.y = tmp_a * tmp_p1.x + tmp_b1;
    
    tmp_b2 = p2.y - tmp_a * p2.x;
    tmp_p2.x = - (b - tmp_b2) / (a - tmp_a);
    tmp_p2.y = tmp_a * tmp_p2.x + tmp_b2;
    
//    cout << "centroid:" << centroid << ", rep1:" << tmp_p1 << ", rep2:" << tmp_p2 << endl;
    
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

bool TextDetector::isSimilar(Scalar c1, Scalar c2)
{
    Scalar diff = c1 - c2;
    double dist = sqrt(diff[0]*diff[0]+diff[1]*diff[1]+diff[2]*diff[2]);
    if (dist <= 50) return true;
    
    return false;
}

bool TextDetector::isLeftContained(cv::Rect s, cv::Rect l)
{
    cv::Rect interrect = *intersect(s, l);
    if ((double)interrect.area() / s.area() == 1) return true;
    else return false;
}

bool TextDetector::isSimilarGradient(Text& text, Object& obj)
{
    if (text.gradients.size() == 0) return true;
    
    cv::Point diff = obj.centroid - text.objects[text.objects.size()-1].centroid;
    double theta = atan2(-diff.y, diff.x);
    double diff_t = fabs(text.gradient - theta);
    if (diff_t > M_PI) diff_t = 2*M_PI - diff_t;
    if (diff_t > M_PI_2) diff_t = M_PI - diff_t;
    
    cout << "Gradient) text:" << text.gradient << ", obj:" << theta << ", diff:" << diff_t << endl;
    if (diff_t < M_PI_3_8) return true;
    else return false;
}

cv::Rect* TextDetector::intersect(const cv::Rect& rect1, const cv::Rect& rect2)
{
    cv::Rect *rect = new cv::Rect(0,0,0,0);
    
    double sx = max(rect1.x, rect2.x);
    double sy = max(rect1.y, rect2.y);
    double ex = min(rect1.x+rect1.width, rect2.x+rect2.width);
    double ey = min(rect1.y+rect1.height, rect2.y+rect2.height);
    
    double w = ex - sx, h = ey - sy;
    if (w > 0 && h > 0) {
        delete rect;
        rect = new cv::Rect(sx, sy, w, h);
    }
    
    return rect;
}
