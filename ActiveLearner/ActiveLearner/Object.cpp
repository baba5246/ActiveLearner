
#include "Object.h"

#define GRID_SIZE   8

#pragma mark -
#pragma mark Inline Methods

inline bool isNotFullIn(Point o, Size s, int x, int y)
{
    return x<=o.x || x>=o.x+s.width-1 || y<=o.y || y>=o.y+s.height-1;
}

inline double distPoints(Point p1, Point p2)
{
    return sqrt(p1.x*p2.x + p1.y*p2.y);
}

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
    aspectRatio = (double)MIN(width, height) / MAX(width, height);
    size = Size(width, height);
    if (centroid.x < 0) centroid = Point((origin.x+width)*0.5f, (origin.y+height)*0.5f);
    
    innerAreaMap = Mat_<int>(height, width);
    innerAreaMap = 0;
    
    computeTLRB();
    
    computeContourArea(GRID_SIZE);
    areaRatio = (double)contourArea / srcSize.area();

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


void Object::computeFeatures(Mat& srcImage)
{
    computeStrokeWidth();
    computeFromInnerAreaMap(srcImage);
    computeEchar();
    
    areaRatio = (double)innerArea / rectArea;
    longLengthRatio = longLengthRatio / MAX(srcSize.width, srcSize.height);
}

void Object::computeStrokeWidth()
{
    int mcount = 0, pcount = 0;
    double mtemp = 0, ptemp = 0, sw = 0, var = 0, mave = 0, pave = 0, mvar = 0, pvar = 0;
    vector<double> msws, psws, sws;
    for (int i = 0; i < contourPixels.size(); i++)
    {
        if (mCorrPairPixels[i].x>0 || mCorrPairPixels[i].y>0) {
            mtemp = distPoints(contourPixels[i], mCorrPairPixels[i]);
            msws.push_back(mtemp);
            mave += mtemp;
            mcount++;
        }
        if (pCorrPairPixels[i].x>0 || pCorrPairPixels[i].y>0) {
            ptemp = distPoints(contourPixels[i], pCorrPairPixels[i]);
            psws.push_back(ptemp);
            pave += ptemp;
            pcount++;
        }
    }
    mave /= mcount;
    pave /= pcount;
    for (int i = 0; i < msws.size(); i++) {
        mvar += pow((mave - msws[i]), 2);
    }
    for (int i = 0; i < psws.size(); i++) {
        pvar += pow((pave - psws[i]), 2);
    }
    mvar /= mcount;
    pvar /= pcount;
    
    if (mcount > pcount) {
        sws = msws;
        var = mvar;
        gradientType = -1;
        corrPairPixels = mCorrPairPixels;
        corrThetas = mCorrThetas;
        innerArea = mInnerArea;
    }
    else {
        sws = psws;
        var = pvar;
        gradientType = 1;
        corrPairPixels = pCorrPairPixels;
        corrThetas = pCorrThetas;
        innerArea = pInnerArea;
    }
    
    sort(sws.begin(), sws.end());
    if (sws.size()>0) sw = sws[sws.size()/2];
    strokeWidth = sw;
    varStrokeWidth = var;
}

void Object::computeFromInnerAreaMap(Mat& src)
{
    // Lab色空間へ
    Mat lab = src.clone();
    cvtColor(src, lab, CV_RGB2Lab);
    
    //
    double count = 0, srcr = 0, srcg = 0, srcb = 0, labr = 0, labg = 0, labb = 0;
    for (int y = 0; y < innerAreaMap.rows; y++) {
        for (int x = 0; x < innerAreaMap.cols; x++) {
            
            Point p(x, y);
            if (innerAreaMap.at<int>(p) == gradientType)
            {
                count++;
                srcr += ((Scalar)src.at<Vec3b>(p+origin))[2];
                srcg += ((Scalar)src.at<Vec3b>(p+origin))[1];
                srcb += ((Scalar)src.at<Vec3b>(p+origin))[0];
                labr += ((Scalar)lab.at<Vec3b>(p+origin))[2];
                labg += ((Scalar)lab.at<Vec3b>(p+origin))[1];
                labb += ((Scalar)lab.at<Vec3b>(p+origin))[0];
            }
        }
    }
    
    if (count > 0) {
        color = CV_RGB(srcr/count, srcg/count, srcb/count);
        labcolor = CV_RGB(floor(labr/count), floor(labg/count), floor(labb/count));
        innerArea = count;
    } else {
        color = CV_RGB(0, 0, 0);
        labcolor = CV_RGB(0, 0, 0);
        innerArea = 0;
    }
}

void Object::computeEchar()
{
    double temp = 0, adgd = 0;
    double alpha = 0.6;
    int fcpcount = 0, count = 0;
    
    
    for (int j = 0; j < thetas.size(); j++)
    {
        if (corrPairPixels[j].x < 0 || corrPairPixels[j].y < 0)
            continue;
        
        if (fabs(thetas[j]) <= M_PI && fabs(corrThetas[j]) <= M_PI)
        {
            temp = fabs(thetas[j] - corrThetas[j]);
            if (temp > M_PI) temp = 2 * M_PI - temp;
            adgd += temp;
            if (temp >= M_PI * alpha) fcpcount++;
        }
        
        count++;
    }
    
    if (count > 0)
    {
        // ADGDとFCPの算出
        Gangle = adgd / count;
        Fcorr = (double)fcpcount / count;
        // Echarの算出
        Echar = (Gangle / M_PI + Fcorr) / 2;
    }
    
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
