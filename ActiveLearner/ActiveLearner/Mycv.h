#pragma once

#include <iostream>
#include <sstream>
#include "Object.h"
#include "MSERegion.h"
#include "Text.h"
#include "Draw.h"

using namespace std;
using namespace cv;

class Mycv
{
public:
    // コンストラクタ
    Mycv(const string& filepath);
    ~Mycv();
    
    // Interface Methods
    void detector(vector<Object>& objects);
    
    // Image Processing Methods
    void grayscale(const Mat& src, Mat& dst);
    void unsharpMasking(const Mat& src, Mat& dst, float k);
    void canny(const Mat &src, Mat &dst);
    void contours(const Mat &src, vector<vector<cv::Point> > &contours, vector<cv::Vec4i>& hierarchy, int mode, int method);
    void sobelFiltering(const Mat& graySrc, Mat_<double>& gradients);
    
    // Contour Interpolating Methods
    void MSERs(const Mat& src, vector<MSERegion>& msers);
    void createObjects(const vector<vector<cv::Point> > &contours, vector<Object> &objects);
    void mergeApartContours(vector<Object>& objects, vector<MSERegion>& msers);
    void mergeIncludedObjects(vector<Object>& objects);
    
    // Compute Feature Methods
    void gradientOfObjects(vector<Object>& objects, const Mat_<double>& gradients);
    bool isPositiveDirection(Object& object);
    Scalar getColor(int x, int y);
    void findCorrPairs(vector<Object>& objects, const Mat& gradients);
    void gradientOfCorrPairs(vector<Object>& objects, const Mat_<double>& gradients);
    void computeEchar(vector<Object>& objects);
    void computeStrokeWidth(vector<Object>& objects);
    
    void setFeatures(vector<Object>& objects);
    
    // Grouping Methods
    void detectTexts(vector<Object>& objects, vector<Text>& texts);
    void groupingObjects(Text& text, Object& obj, vector<double>& distance, vector<Object>& objects);
    void findNeighbors(vector<Object*>& neighbors, Object& obj, vector<double>& distance, vector<Object>& objects);
    void adding(Text& text, vector<Object*>& neighbors);
    bool isSimilar(Scalar c1, Scalar c2);
    
private:
    Mat srcImage;
    IplImage srcIplImage;
    string filename;
    long id_count = 0;
    long recursive_count = 0;
    
    vector<string> split(const string& str, char delim);
    int** createImageTable(const vector<Object>& objects);
    cv::Rect* intersect(const cv::Rect& rect1, const cv::Rect& rect2);
    
    double distanceWithNearestNeighbor(Object& obj, vector<Object>& objects);
    double distanceOfObjects(const Object& obj1, const Object& obj2);
    int patternOfRadian(const double radian);
    double distanceOfPoints(const cv::Point& p1, const cv::Point& p2);
    cv::Point pointApartFromCentroid(const cv::Point& centroid, const cv::Point& p1, const cv::Point& p2, const double a);
    cv::Point findRepresentativePoint(const Object& obj, const double a, const int pattern);
    
};
