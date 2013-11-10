
#include "Mycv.h"

#pragma mark -
#pragma mark Constructor

Mycv::Mycv(const string& filepath)
{
    filename = filepath;
    srcImage = imread(filepath, CV_LOAD_IMAGE_COLOR);
}

#pragma mark -
#pragma mark Interface Methods

void Mycv::detector()
{
    // Detect contours
    Mat imgGray, imgCanny;
    Mycv::grayscale(srcImage, imgGray);
    Mycv::canny(imgGray, imgCanny);
    
    cv::vector<cv::Vec4i> hierarchy;
    cv::vector<cv::vector<cv::Point> > contours;
    Mycv::contours(imgCanny, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    
    
    // Create objects
    vector<Object> objects;
    Mycv::createObjects(contours, objects);
    
    
    // Detect MSERs
    Mat imgUnsharp;
    Mycv::unsharpMasking(srcImage, imgUnsharp, 1);
    
    vector<MSERegion> msers;
    Mycv::MSERs(imgUnsharp, msers);
    

    Mycv::mergeApartContours(objects, msers);
 
    Draw::drawObjects(srcImage, objects);
}


#pragma mark -
#pragma mark Basic Processing Methods

// Gray Scale
void Mycv::grayscale(const Mat &src, Mat &dst)
{
    cvtColor(src, dst, COLOR_BGR2GRAY);
}

// Canny Edge Detector
void Mycv::canny(const Mat &src, Mat &dst)
{
    cv::Canny(src, dst, 50, 200);
}

// Contours
void Mycv::contours(const Mat &src, vector<vector<cv::Point>> &contours, vector<cv::Vec4i>& hierarchy, int mode, int method)
{
    // "Each contour is stored as a vector of points."
    cv::findContours(src, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);

}

// Unsharp Masking
void Mycv::unsharpMasking(const Mat& src, Mat& dst, float k)
{
    Mat blurred;
    double sigma = 1, threshold = 5, amount = 1;
    
    GaussianBlur(src, blurred, Size(), sigma);
    Mat lowConstrastMask = abs(src - blurred) < threshold;
    dst = src*(1+amount) + blurred*(-amount);
    src.copyTo(dst, lowConstrastMask);
}

// MSER
void Mycv::MSERs(const Mat& src, vector<MSERegion>& msers)
{
    int delta = 5, min_area = 0, max_area = 60000;
    float max_variation = 0.25f, min_diversity = 0.20f;
    int max_evolution = 200;
    double area_threshold = 1.01, min_margin = 0.0030;
    int edge_blur_size = 5;
    
    vector<vector<cv::Point>> mser_features;
    cv::MSER mser(delta, min_area, max_area, max_variation, min_diversity,
                  max_evolution, area_threshold, min_margin, edge_blur_size);
    mser(src, mser_features, Mat());
    
    for (int i = 0; i < mser_features.size(); i++)
    {
        MSERegion mseregion(mser_features[i]);
        msers.push_back(mseregion);
    }
}


// Sobel Filtering


#pragma mark -
#pragma mark Feature Detection Processing Methods

// Gradients of Objects


// Create Objects and Set Contours
void Mycv::createObjects(const vector<vector<cv::Point>>& contours, vector<Object>& objects)
{
    int imgArea = srcImage.rows*srcImage.cols;
    double ratio = 0;
    for (int i = 0; i < contours.size(); i++) {
        Object obj = *new Object(filename, contours[i]);
        ratio = (double)obj.contourArea / obj.rectArea;
        
        printf("%d, %d, %d, %d\n", obj.rectArea>30, obj.rectArea < imgArea*0.25f, ratio > 0.4f, obj.aspectRatio < 5);
        if (obj.rectArea>30 && obj.rectArea < imgArea*0.25f &&
            ratio > 0.4f && obj.aspectRatio < 5)
        {
            objects.push_back(obj);
        }
    }
}

// Compare MBR between Contours and MSER
void Mycv::mergeApartContours(vector<Object>& objects, vector<MSERegion>& msers)
{
    cv::Rect objRect, mserRect, interRect;
    double objRectRatio = 0, mserRectRatio = 0;
    double objRatioThre = 0.5f, mserThreshold = 0.3f;
    
    // ObjectとMSERの矩形の重なり割合でObjectが含むMSERを取得する
    for (int i = 0; i < objects.size(); i++)
    {
        objRect = objects[i].rect;
        objRectRatio = 0;
        
        for (int j = 0; j < msers.size(); j++)
        {
            mserRect = msers[j].rect;
            interRect = *intersect(objects[i].rect, msers[j].rect);
            
            objRectRatio = (double)interRect.area() / objRect.area();
            mserRectRatio = (double)interRect.area() / mserRect.area();
            
            if (objRectRatio > objRatioThre)
            {
                if (abs(objRectRatio - 1) < abs(objects[i].rectRatio - 1)
                    && mserRectRatio > mserThreshold)
                {
                    objects[i].rectRatio = objRectRatio;
                    objects[i].mserIndex = j;
                    msers[j].objIndexes.push_back(i);
                }
            }
        }
    }
    
    vector<int> removeIndexes;
    int baseIndex = 0, tempIndex = 0;
    for (int i = 0; i < msers.size(); i++)
    {
        for (int j = 0; j < msers[i].objIndexes.size(); j++)
        {
            baseIndex = msers[i].objIndexes[0];
            tempIndex = msers[i].objIndexes[j];
            if (j == 0) continue;
            Object *baseObj = &objects[baseIndex];
            Object *tempObj = &objects[tempIndex];
            baseObj->mergeObject(*tempObj);
            removeIndexes.push_back(tempIndex);
        }
    }
    
    sort(removeIndexes.begin(), removeIndexes.end());
    vector<int>::iterator  uniqued = unique(removeIndexes.begin(), removeIndexes.end());
	removeIndexes.erase(uniqued, removeIndexes.end());
    
    for (int i = (int)removeIndexes.size()-1; i > -1; i--)
    {
        objects.erase(objects.begin() + removeIndexes[i]);
    }
    
    removeIndexes.clear();
}


// Inclusion Relationship


// Merge Hole Objects


// Find Corresponding Pairs


// Compute Features



#pragma mark -
#pragma mark Utility Methods
cv::Rect* Mycv::intersect(const cv::Rect& rect1, const cv::Rect& rect2)
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


