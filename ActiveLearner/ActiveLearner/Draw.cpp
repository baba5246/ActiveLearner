
#include "Draw.h"

void Draw::drawImage(const Mat& src)
{
    if(!src.data) {
        cout <<  "Could not open or find the image" << endl ;
    } else {
        namedWindow( "Display window", CV_WINDOW_AUTOSIZE );
        imshow( "Display window", src);
        waitKey(4000);
        destroyWindow("Display window");
    }
}

void Draw::draw(const Mat& src)
{
    drawImage(src);
}

void Draw::drawContours(const Mat& src, const vector<vector<Point> >& contours, const vector<cv::Vec4i>& hierarchy)
{
    //srand((unsigned int)time(NULL));
    
    Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
    int idx = 0;
    for( ; idx >= 0; idx = hierarchy[idx][0] )
    {
        Scalar color( rand()&255, rand()&255, rand()&255 );
        cv::drawContours(dst, contours, idx, color, CV_FILLED, 8, hierarchy);
    }
    
    drawImage(dst);
}

void Draw::drawMSERs(const Mat& src, const vector<vector<Point> >& mser_features)
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

void Draw::drawObjects(const Mat& src, vector<Object*>& objects)
{
    Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
    
    for (int i = 0; i < objects.size(); i++)
    {
        vector<cv::Point> pixels = objects[i]->contourPixels;
        Scalar color = CV_RGB(rand()&255, rand()&255, rand()&255);
        if (objects[i]->colors.size()>0) color = objects[i]->color;
        
        for (int j = 0; j < pixels.size(); j++)
        {
            circle(dst, pixels[j], 0.1f, color);
        }
    }
    
    drawImage(dst);
}

void Draw::drawGradients(const Mat& src, const Mat_<double>& gradients)
{
    Mat dst = Mat::zeros(gradients.rows, gradients.cols, CV_8UC3);
    
    double g = 0;
    Scalar color;
    for (int y = 0; y < dst.rows; y++)
    {
        for (int x = 0; x < dst.cols; x++)
        {
            g = gradients.at<double>(y, x);
            color = *colorOfRadian(g);
            int a  = (int)(y * dst.step + x * dst.channels());
            dst.data[a+0] = color[0];
            dst.data[a+1] = color[1];
            dst.data[a+2] = color[2];
        }
    }
    
    drawImage(dst);
}

void Draw::drawGradients(const vector<Object*>& objects, const Mat_<double>& gradients)
{
    Mat dst = Mat::zeros(gradients.rows, gradients.cols, CV_8UC3);
    
    cv::Point p;
    double g = 0;
    Scalar color;
    for (int i = 0; i < objects.size(); i++) {
        for (int j = 0; j < objects[i]->contourPixels.size(); j++) {
            p = objects[i]->contourPixels[j];
            g = gradients.at<double>(p.y, p.x);
            color = *colorOfRadian(objects[i]->thetas[j]);
            int a  = (int)(p.y * dst.step + p.x * dst.channels());
            dst.data[a+0] = color[0];
            dst.data[a+1] = color[1];
            dst.data[a+2] = color[2];
        }
    }
    
    drawImage(dst);
}

cv::Scalar* Draw::colorOfRadian(double radian)
{
    cv::Scalar *color;
    
    if (radian <= M_PI && radian > M_PI_7_8) color = new cv::Scalar(0, 0, 255);
    else if (radian <= M_PI_7_8 && radian > M_PI_5_8) color = new cv::Scalar(0, 127, 255);
    else if (radian <= M_PI_5_8 && radian > M_PI_3_8) color = new cv::Scalar(0, 255, 255);
    else if (radian <= M_PI_3_8 && radian > M_PI_1_8) color = new cv::Scalar(0, 255, 0);
    else if (radian <= M_PI_1_8 && radian > -M_PI_1_8) color = new cv::Scalar(255, 255, 0);
    else if (radian <= -M_PI_1_8 && radian > -M_PI_3_8) color = new cv::Scalar(255, 0, 0);
    else if (radian <= -M_PI_3_8 && radian > -M_PI_5_8) color = new cv::Scalar(255, 0, 127);
    else if (radian <= -M_PI_5_8 && radian > -M_PI_7_8) color = new cv::Scalar(127, 0, 255);
    else color = new cv::Scalar(0, 0, 255);
    
    return color;
}

void Draw::drawEchars(const Mat& src, const vector<Object*>& objects)
{
    Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
    
    for (int i = 0; i < objects.size(); i++)
    {
        vector<cv::Point> pixels = objects[i]->contourPixels;
        Scalar color;
        
        //if (objects[i].Echar < 0.75) continue;
        
        if (objects[i]->isPositive) color = CV_RGB(objects[i]->Echar*BRIGHTNESS, objects[i]->Echar*BRIGHTNESS, 0);
        else color = CV_RGB(0, objects[i]->Echar*BRIGHTNESS, objects[i]->Echar*BRIGHTNESS);
        
        vector<cv::Point> corrPixels = objects[i]->corrPairPixels;
        for (int j = 0; j < pixels.size(); j++)
        {
            circle(dst, pixels[j], 0.1f, color);
            if (corrPixels.size()>0 && corrPixels[j].x >= 0 && corrPixels[j].y >= 0) line(dst, pixels[j], corrPixels[j], color);
        }
    }
    
    drawImage(dst);
}

void Draw::drawGradientLine(const Mat& src, const vector<Object*>& objects, double a, double b)
{
    Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
    
    int count = 0;
    
    for (int i = 0; i < objects.size(); i++)
    {
        count = 0;
        vector<cv::Point> pixels = objects[i]->contourPixels;
        vector<cv::Point> corrPixels = objects[i]->corrPairPixels;
        Scalar color = CV_RGB(rand()&255, rand()&255, rand()&255);
        
        for (int j = 0; j < pixels.size(); j++)
        {
            circle(dst, pixels[j], 0.1f, color);
            if (corrPixels.size()>0 && corrPixels[j].x >= 0 && corrPixels[j].y >= 0)
                line(dst, pixels[j], corrPixels[j], color);
        }
    }
    
    drawImage(dst);
}

void Draw::drawSurroundings(const Mat& src, const vector<Object*>& objects)
{
    Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
    
    for (int i = 0; i < objects.size(); i++)
    {
        vector<cv::Point> pixels = objects[i]->surroundings;
        vector<double> thetas = objects[i]->surrThetas;
        Scalar color;
        
        for (int j = 0; j < pixels.size(); j++)
        {
            color = *colorOfRadian(thetas[j]);
            circle(dst, pixels[j], 0.1f, color);
        }
    }
    
    drawImage(dst);
}

void Draw::drawText(const cv::Mat &src, Text *&text)
{
    Mat dst = src.clone();
    
    srand((unsigned int)1);
    Scalar red = CV_RGB(BRIGHTNESS,0,0);
    Scalar blue = CV_RGB(0, 0, BRIGHTNESS);
    rectangle(dst, text->rect, red, 3);
    
    for (int j = 0; j < text->objects.size(); j++) {
        if (j == 0) circle(dst, text->objects[j]->centroid, 3, red, 3);
        else if (j == text->focusedIndex) circle(dst, text->objects[j]->centroid, 3, blue);
        else circle(dst, text->objects[j]->centroid, 3, red);
        
        if (text->originIndexes[j] >= 0) {
            line(dst, text->objects[j]->centroid, text->objects[text->originIndexes[j]]->centroid, red);
        }
    }
    
    drawImage(dst);
}

void Draw::drawTexts(const Mat& src, const vector<Text*>& texts)
{
    Mat dst = Mat(src);
    
    srand((unsigned int)1);
    Scalar color;
    for (int i = 0; i < texts.size(); i++) {
        color = CV_RGB((double)rand() / RAND_MAX * BRIGHTNESS,
                       (double)rand() / RAND_MAX * BRIGHTNESS,
                       (double)rand() / RAND_MAX * BRIGHTNESS);
        rectangle(dst, texts[i]->rect, color, 3);
        
        for (int j = 0; j < texts[i]->objects.size(); j++) {
            if (j == 0) circle(dst, texts[i]->objects[j]->centroid, 3, color, 3);
            else circle(dst, texts[i]->objects[j]->centroid, 3, color, 1);
        }
    }
    
    drawImage(dst);
}