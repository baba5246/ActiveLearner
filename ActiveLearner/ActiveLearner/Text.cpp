
#include "Text.h"

#pragma mark -
#pragma mark Contructors

Text::Text()
{
    
}

Text::Text(string name, Object& obj)
{
    filename = name;
    add(obj);
}


#pragma mark -
#pragma mark Public Methods

void Text::add(Object& obj)
{
    objects.push_back(obj);
    computeGradient(obj);
    computeProperties();
}

bool Text::areAllGrouped()
{
    for (int i = 0; i < objects.size(); i++) {
        if (!objects[i].grouped) return false;
    }
    
    return true;
}


#pragma mark -
#pragma mark Private Methods

void Text::computeProperties()
{
    int minx = INFINITY, miny = INFINITY, maxx = 0, maxy = 0;
    for (int i = 0; i < objects.size(); i++)
    {
        if (minx > objects[i].rect.tl().x) minx = objects[i].rect.tl().x;
        if (miny > objects[i].rect.tl().y) miny = objects[i].rect.tl().y;
        if (maxx < objects[i].rect.br().x) maxx = objects[i].rect.br().x;
        if (maxy < objects[i].rect.br().y) maxy = objects[i].rect.br().y;
    }
    width = maxx - minx + 1;
    height = maxy - miny + 1;
    rect = cv::Rect(minx, miny, width, height);
    aspectRatio = (double) width / height;
    computeColor();
}

void Text::computeColor()
{
    double r = 0, g = 0, b = 0;
    long length = objects.size();
    
    if (length != 0) {
    
        for (int i = 0; i < length; i++) {
            r += objects[i].color[0];
            g += objects[i].color[1];
            b += objects[i].color[2];
        }
        
        r /= length;
        g /= length;
        b /= length;
    }
    
    color = Scalar(r, g, b);
}

void Text::computeGradient(Object& obj)
{
    if (objects.size() < 2) return;
    
    cv::Point diff = obj.centroid - objects[objects.size()-2].centroid;
    double theta = atan2(-diff.y, diff.x);
    gradients.push_back(theta);
    
    double temp = 0;
    for (int i = 0; i < gradients.size(); i++) temp += gradients[i];
    gradient = temp / gradients.size();
    
}
