#pragma once

#include <iostream>
#include <sstream>
#include <pthread.h>
#include "Mycv.h"
#include "Object.h"
#include "MSERegion.h"
#include "SWTObject.h"
#include "Draw.h"
#include "TextDetector.h"

using namespace std;
using namespace cv;

class ObjectDetector {
    
public:
    
private:
    
    cv::Mat srcImage;
    IplImage srcIplImage;
    string filepath, filename;
    long id_count = 0;
    int srcW = 0, srcH = 0;
    
public:
    
    // Contructors
    ObjectDetector();
    ObjectDetector(const string& path);
    ~ObjectDetector();
    
    // Interface Methods
    void detect(vector<Object*>& objects);
    
private:
    
    // Contour Interpolating Methods
    void createObjects(const vector<vector<cv::Point> >& contours, vector<Object*>& objects);
    void mergeApartContours(vector<Object*>& objects, vector<MSERegion>& msers);
    void mergeIncludedObjects(vector<Object*>& objects);
    
    // SWT Methods
    void createSWTObjects(vector<SWTObject>& swtobjects, const Mat& swt, vector<vector<cv::Point> >& components);
    
    // Compute Feature Methods
    void gradientOfObjects(vector<Object*>& objects, const Mat_<double>& gradients);
    bool isPositiveDirection(Object* object);
    Scalar getColor(int x, int y);
    void findCorrPairs(vector<Object*>& objects, const Mat& gradients);
    void findPairs(Object*& object, int index, int type, Mat_<int> table);
    void gradientOfCorrPairs(vector<Object*>& objects, const Mat_<double>& gradients);
    
    void objectFiltering(vector<Object*>& dst_objects, vector<Object*>& src_objects);
    void setFeatures(vector<Object*>& objects);
    
    Mat_<int> createImageTable(const vector<Object*>& objects);
    cv::Rect* intersect(const cv::Rect& rect1, const cv::Rect& rect2);
    
    Scalar computeColor(const Mat& srcImage, const vector<cv::Point>& region);
    
    
};

