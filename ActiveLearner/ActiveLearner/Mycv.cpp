
#include "Mycv.h"

#pragma mark -
#pragma mark Constructor

Mycv::Mycv(Mat& src)
{
    srcImage = src;
    srcIplImage = srcImage;
}

#pragma mark -
#pragma mark Basic Processing Methods

// Gray Scale
void Mycv::grayscale(const Mat &src, Mat &dst)
{
    Mat temp;
    cvtColor(src, temp, COLOR_BGR2BGR565);
    cvtColor(temp, dst, COLOR_BGR5652GRAY);
}


void Mycv::grayscale(const Mat &src, Mat &dst, int type)
{
    for (int y = 0; y < src.rows; y++) {
        for (int x = 0; x < src.cols; x++) {
            cv::Vec3b bgr = src.at<cv::Vec3b>(y, x);
            dst.at<Vec3b>(y, x) = cv::Vec3b(bgr[type],bgr[type],bgr[type]);
        }
    }
}

// Canny Edge Detector
void Mycv::canny(const Mat &src, Mat &dst)
{
    cv::Canny(src, dst, 50, 200);
}

void Mycv::mergeEdges(const Mat &r, const Mat &g, const Mat &b, Mat &dst)
{
    for (int y = 0; y < r.rows; y++) {
        for (int x = 0; x < r.cols; x++) {
            int rp = r.at<int>(y, x);
            int gp = g.at<int>(y, x);
            int bp = b.at<int>(y, x);
            
            if (rp == gp || rp == bp) {
                dst.at<int>(y, x) = rp;
            } else if (gp == bp) {
                dst.at<int>(y, x) = gp;
            }
        }
    }
}

// Contours
void Mycv::contours(const Mat &src, vector<vector<cv::Point> > &contours, vector<cv::Vec4i>& hierarchy, int mode, int method)
{
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
    int delta = 10, min_area = 0, max_area = 100000;
    float max_variation = 0.25f, min_diversity = 0.20f;
    int max_evolution = 200;
    double area_threshold = 1.01, min_margin = 0.0030;
    int edge_blur_size = 5;
    
    vector<vector<cv::Point> > mser_features;
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
void Mycv::sobelFiltering(const Mat& graySrc, Mat_<double>& gradients)
{
    gradients += INFINITY;
    Mat grad_x = Mat::zeros(graySrc.rows, graySrc.cols, CV_8U);
    Mat grad_y = Mat::zeros(graySrc.rows, graySrc.cols, CV_8U);
    
    Point aroundx[6] = { Point(-1, -1), Point(-1, 0), Point(-1, 1), Point(1, -1), Point(1, 0), Point(1, 1) };
    Point aroundy[6] = { Point(-1, -1), Point(0, -1), Point(1, -1), Point(-1, 1), Point(0, 1), Point(1, 1) };
    
    int dx = 0, dy = 0;
    double g = 0;
    for (int y = 0; y < graySrc.rows; y++)
    {
        for (int x = 0; x < graySrc.cols; x++)
        {
            if (x < 1 || y < 1 || x > graySrc.cols-2 || y > graySrc.rows-2) continue;
            
            Point p(x, y);
            Point xp[] = { p+aroundx[0], p+aroundx[1], p+aroundx[2], p+aroundx[3], p+aroundx[4], p+aroundx[5] };
            Point yp[] = { p+aroundy[0], p+aroundy[1], p+aroundy[2], p+aroundy[3], p+aroundy[4], p+aroundy[5] };
            unsigned long xi[6], yi[6];
            for (int i = 0; i < 6; i++) xi[i] = xp[i].y*graySrc.step+xp[i].x*graySrc.channels();
            for (int i = 0; i < 6; i++) yi[i] = yp[i].y*graySrc.step+yp[i].x*graySrc.channels();
            
            dx = graySrc.data[xi[0]] + 2 * graySrc.data[xi[1]] + graySrc.data[xi[2]]
                    - graySrc.data[xi[3]] - 2 * graySrc.data[xi[4]] - graySrc.data[xi[5]];
            dy = graySrc.data[yi[0]] + 2 * graySrc.data[yi[1]] + graySrc.data[yi[2]]
                    - graySrc.data[yi[3]] - 2 * graySrc.data[yi[4]] - graySrc.data[yi[5]];
            dy *= -1;
            g = atan2(dy, dx);
            gradients.at<double>(y, x) = g;
        }
    }
}

void Mycv::decreaseColors(const Mat& src, Mat& dst)
{
    const int cluster_count = 8; /* number of cluster */
    
    // (2)reshape the image to be a 1 column matrix
    Mat points;
    src.convertTo(points, CV_32FC3);
    points = points.reshape(3, src.rows*src.cols);
    
    // (3)run k-means clustering algorithm to segment pixels in RGB color space
    Mat_<int> clusters(points.size(), CV_32SC1);
    Mat centers;
    kmeans(points, cluster_count, clusters,
           cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0), 1, KMEANS_PP_CENTERS, centers);
    
    // (4)make a each centroid represent all pixels in the cluster
    dst = Mat(src.size(), src.type());
    MatIterator_<Vec3f> itf = centers.begin<Vec3f>();
    MatIterator_<Vec3b> itd = dst.begin<Vec3b>(), itd_end = dst.end<Vec3b>();
    for(int i=0; itd != itd_end; ++itd, ++i) {
        Vec3f color = itf[clusters(1,i)];
        (*itd)[0] = saturate_cast<uchar>(color[0]);
        (*itd)[1] = saturate_cast<uchar>(color[1]);
        (*itd)[2] = saturate_cast<uchar>(color[2]);
    }
    
    // (5)show source and destination image, and quit when any key pressed
    namedWindow("src_img", CV_WINDOW_AUTOSIZE);
    imshow("src_img", src);
    namedWindow("dst_img", CV_WINDOW_AUTOSIZE);
    imshow("dst_img", dst);
    waitKey(0);
}


Mycv::~Mycv()
{
    
}

