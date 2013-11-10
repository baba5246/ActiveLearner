
#include "Object.h"

#define GRID_SIZE   8

#pragma mark -
#pragma mark Constructor

Object::Object(const string& filepath, const vector<cv::Point>& contour)
{
    filename = filepath;
    contourPixels = contour;
    computeProperties();
}


#pragma mark -
#pragma mark Compute Properties Methods

void Object::computeProperties()
{
    rect = boundingRect(contourPixels);
    
    origin = Point(rect.x, rect.y);
    width = rect.width;
    height = rect.height;
    size = Size(width, height);
    centroid = Point((origin.x+width)*0.5f, (origin.y+height)*0.5f);
    
    rectArea = rect.area();
    computeContourArea(GRID_SIZE);
    
    computeColor();
}

void Object::computeContourArea(int gridSize)
{
    int64 area = 0;
    bool grid[gridSize*gridSize];
    
    for (int i = 0; i < gridSize*gridSize; i++) grid[i] = false;
    
    int x = 0, y = 0, index = 0;
    int gridWidth = width / gridSize + 1, gridHeight = height / gridSize + 1;
    for (int i = 0; i < contourPixels.size(); i++)
    {
        cv::Point p = contourPixels[i];
        x = (int)floor((double)(p.x - origin.x) / gridWidth);
        y = (int)floor((double)(p.y - origin.y) / gridHeight);
        index = gridSize * y + x;
        if (!grid[index]) grid[index] = true;
    }
    
    for (int i = 0; i < sizeof(grid); i++){
        if (grid[i]) area++;
    }
    
    contourArea = (int)area * gridWidth * gridHeight;
}


void Object::computeColor()
{
    
}



#pragma mark -
#pragma mark Inclusion Relationship Methods

bool isParentOf(Object obj)
{
    bool isParent = false;
    
    
    return isParent;
}

bool isChildOf(Object obj)
{
    bool isChild = false;
    
    
    return isChild;
}

void Object::mergeObject(Object obj)
{
    contourPixels.insert(contourPixels.end(), obj.contourPixels.begin(), obj.contourPixels.end());
    computeProperties();
}


Object::~Object()
{
    
}
