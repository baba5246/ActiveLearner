
#include "Draw.h"

#define BRIGHTNESS  256

void Draw::drawImage(const Mat& src)
{
    if(!src.data) {
        cout <<  "Could not open or find the image" << endl ;
    } else {
        namedWindow( "Display window", CV_WINDOW_AUTOSIZE );
        imshow( "Display window", src);
        waitKey(0);
    }
}

void Draw::draw(const Mat& src)
{
    drawImage(src);
}

void Draw::drawContours(const Mat& src, const vector<vector<Point>>& contours, const vector<cv::Vec4i>& hierarchy)
{
    srand((unsigned int)time(NULL));
    
    Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
    int idx = 0;
    for( ; idx >= 0; idx = hierarchy[idx][0] )
    {
        Scalar color( rand()&255, rand()&255, rand()&255 );
        cv::drawContours(dst, contours, idx, color, CV_FILLED, 8, hierarchy );
    }
    
    drawImage(dst);
}

void Draw::drawMSERs(const Mat& src, const vector<vector<Point>>& mser_features)
{
    Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
    for (int i = 0; i < mser_features.size(); i++)
    {
        Scalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
        for (int j = 0; j < mser_features[i].size(); j++)
        {
            circle(dst, mser_features[i][j], 1, color);
        }
        ellipse(dst, fitEllipse(mser_features[i]), CV_RGB(255, 0, 0));
    }
    
    drawImage(dst);
}

void Draw::drawObjects(const Mat& src, const vector<Object> objects)
{
    Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
    
    for (int i = 0; i < objects.size(); i++)
    {
        vector<cv::Point> pixels = objects[i].contourPixels;
        Scalar color = CV_RGB(rand()&255, rand()&255, rand()&255);
        for (int j = 0; j < pixels.size(); j++)
        {
            circle(dst, pixels[j], 0.1f, color);
        }
    }
    
    drawImage(dst);
}

