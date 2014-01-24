
#include "Object.h"

#define GRID_SIZE   8

#pragma mark -
#pragma mark Constructor
Object::Object()
{
    
}

Object::Object(const string Id, const string& path, const vector<cv::Point>& contour, const cv::Size size)
{
    ID = Id;
    filepath = path;
    filename = filepath.substr(filepath.find_last_of("/")+1);
    contourPixels = contour;
    srcSize = size;
    computeProperties();
}


#pragma mark -
#pragma mark Compute Properties Methods

void Object::computeProperties()
{
    if (contourPixels.size() == 0) return;
    
    uniqueContour();
    rect = boundingRect(contourPixels);
    minEnclosingCircle(contourPixels, center, r);
    rectArea = rect.area();
    rectRatio = (double)rectArea / srcSize.area();
    
    origin = Point(rect.x, rect.y);
    width = rect.width;
    height = rect.height;
    longLength = MAX(width, height);
    longLengthRatio = longLength / MAX(srcSize.width,srcSize.height);
    aspectRatio = (double)MIN(width, height) / MAX(width, height);
    size = Size(width, height);
    if (centroid.x < 0) centroid = Point((origin.x+width)*0.5f, (origin.y+height)*0.5f);
    
    innerAreaMap = Mat_<int>(height, width);
    innerAreaMap = 0;
    
    computeTLRB();
    
    computeContourArea(GRID_SIZE);
    areaRatio = (double)contourArea / srcSize.area();
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

void Object::computeTLRB()
{
    cv::Point p;
    int t = INFINITY, l = INFINITY, r = 0, b = 0;
    for (int i = 0; i < contourPixels.size(); i++) {
        p = contourPixels[i];
        if (p.x < l) {
            l = p.x;
            lp = p;
        } else if (p.x > r) {
            r = p.x;
            rp = p;
        }
        if (p.y < t) {
            t = p.y;
            tp = p;
        } else if (p.y > b) {
            b = p.y;
            bp = p;
        }
    }
}

void Object::computeColor(Mat& src)
{
    // Lab色空間へ
    Mat lab = src.clone();
    cvtColor(src, lab, CV_RGB2Lab);
    
    //
    double count = 0, srcr = 0, srcg = 0, srcb = 0, labr = 0, labg = 0, labb = 0;
    for (int y = 0; y < innerAreaMap.rows; y++) {
        for (int x = 0; x < innerAreaMap.cols; x++) {
            //
            if (innerAreaMap.at<int>(y,x)>0)
            {
                count++;
                srcr += src.at<Vec3b>(y,x)[0];
                srcg += src.at<Vec3b>(y,x)[1];
                srcb += src.at<Vec3b>(y,x)[2];
                labr += lab.at<Vec3b>(y,x)[0];
                labg += lab.at<Vec3b>(y,x)[1];
                labb += lab.at<Vec3b>(y,x)[2];
            }
        }
    }
    
    if (count > 0) {
        color = CV_RGB(floor(srcr/count), floor(srcg/count), floor(srcb/count));
        labcolor = CV_RGB(floor(labr/count), floor(labg/count), floor(labb/count));
    } else {
        color = CV_RGB(0, 0, 0);
        labcolor = CV_RGB(0, 0, 0);
    }
}

void Object::uniqueContour()
{
    int cx = 0, cy = 0;
    
    vector<cv::Point> pixels;
    for (int i = 0; i < contourPixels.size(); i++)
    {
        if (find(pixels.begin(), pixels.end(), contourPixels[i]) != pixels.end())
            continue;
        
        cv::Point p = contourPixels[i];
        pixels.push_back(p);
        
        cx += p.x;
        cy += p.y;
    }
    
    if (pixels.size()>0) centroid = cv::Point(cx/pixels.size(), cy/pixels.size());
    
    contourPixels = pixels;
}

inline double distPoints(Point p1, Point p2)
{
    return sqrt(p1.x*p2.x + p1.y*p2.y);
}

void Object::computeStrokeWidth()
{
    int count = 0;
    double temp = 0, sw = 0;
    vector<double> swidth;
    for (int i = 0; i < contourPixels.size(); i++)
    {
        if (corrPairPixels[i].x>0 && corrPairPixels[i].y>0) {
            temp = distPoints(contourPixels[i], corrPairPixels[i]);
            swidth.push_back(temp);
            sw += temp;
            count++;
        }
    }
//    strokeWidth = sw / count;
    
    sort(swidth.begin(), swidth.end());
    if (swidth.size()>0) sw = swidth[swidth.size()/2];
    strokeWidth = sw;
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
