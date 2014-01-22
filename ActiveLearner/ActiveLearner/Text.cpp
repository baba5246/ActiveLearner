
#include "Text.h"

#pragma mark -
#pragma mark Contructors

Text::Text()
{
    
}

Text::Text(string path, Object*& obj)
{
    filepath = path;
    filename = filepath.substr(filepath.find_last_of("/")+1);
    srcImage = imread(filepath);
    add(obj, 0);
}


#pragma mark -
#pragma mark Public Methods

void Text::add(Object*& obj, double distance)
{
    objects.push_back(obj);
    distances.push_back(distance);
    originIndexes.push_back(focusedIndex);
    computeGradient(obj);
    computeProperties();
    computeAverageDistance();
}

bool Text::areAllFocused()
{
    for (int i = 0; i < objects.size(); i++) {
        if (!objects[i]->didFocused) return false;
    }
    
    return true;
}

bool Text::contains(Object*& obj)
{
    for (int i = 0; i < objects.size(); i++) {
        if (!obj->ID.compare(objects[i]->ID)) return true;
    }
    
    return false;
}


#pragma mark -
#pragma mark Private Methods

void Text::computeProperties()
{
    int minx = INFINITY, miny = INFINITY, maxx = 0, maxy = 0;
    for (int i = 0; i < objects.size(); i++)
    {
        if (minx > objects[i]->rect.tl().x) minx = objects[i]->rect.tl().x;
        if (miny > objects[i]->rect.tl().y) miny = objects[i]->rect.tl().y;
        if (maxx < objects[i]->rect.br().x) maxx = objects[i]->rect.br().x;
        if (maxy < objects[i]->rect.br().y) maxy = objects[i]->rect.br().y;
    }
    width = maxx - minx + 1;
    height = maxy - miny + 1;
    rect = cv::Rect(minx, miny, width, height);
    aspectRatio = (double) MIN(width, height) / MAX(width, height);
    computeColor();
    
}

void Text::computeColor()
{
    double r = 0, g = 0, b = 0;
    long length = objects.size();
    
    if (length != 0) {
    
        for (int i = 0; i < length; i++) {
            r += objects[i]->color[0];
            g += objects[i]->color[1];
            b += objects[i]->color[2];
        }
        
        r /= length;
        g /= length;
        b /= length;
    }
    
    color = Scalar(r, g, b);
}

void Text::computeAverageDistance()
{
    averageDistance = 0;
    if (distances.size() == 0) return;
    
    for (int i = 0; i < distances.size(); i++) {
        averageDistance += distances[i];
    }
    averageDistance /= distances.size();
}

void Text::computeGradient(Object*& obj)
{
    if (objects.size() < 2) return;
    
    cv::Point diff = obj->centroid - objects[objects.size()-2]->centroid;
    double theta = atan2(-diff.y, diff.x);
    gradients.push_back(theta);
    
    double temp = 0;
    for (int i = 0; i < gradients.size(); i++) temp += gradients[i];
    gradient = temp / gradients.size();
    
}
