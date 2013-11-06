
#include "Object.h"

#pragma mark -
#pragma mark Constructor

Object::Object(const string& filepath)
{
    filename = filepath;
}

#pragma mark -
#pragma mark Compute Properties Methods

void Object::computeProperties()
{
    computeCentroid();
    computeContourArea();
    computeColor();
}

void computeCentroid()
{
    int x = 0, y = 0;
    for (int i = 0; i < 5; i++) {
        
    }
}

void computeContourArea()
{
    
}

void computeColor()
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

void mergeObject(Object obj)
{
    
}