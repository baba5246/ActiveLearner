
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
    dst = Mat(src.rows, src.cols, CV_8UC3);
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
    dst = Mat(r.rows, r.cols, CV_8UC1);
    for (int y = 0; y < r.rows; y++) {
        for (int x = 0; x < r.cols; x++) {
            int rp = r.at<uchar>(y, x);
            int gp = g.at<uchar>(y, x);
            int bp = b.at<uchar>(y, x);
            
            if (rp > 0) dst.at<uchar>(y, x) = r.at<uchar>(y, x);
            else if (gp > 0) dst.at<uchar>(y, x) = g.at<uchar>(y, x);
            else if (bp > 0) dst.at<uchar>(y, x) = b.at<uchar>(y, x);
            else dst.at<uchar>(y, x) = 0;
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
    
    cv::Point aroundx[6] = { cv::Point(-1, -1), cv::Point(-1, 0), cv::Point(-1, 1),
                             cv::Point(1, -1), cv::Point(1, 0), cv::Point(1, 1) };
    cv::Point aroundy[6] = { cv::Point(-1, -1), cv::Point(0, -1), cv::Point(1, -1),
                             cv::Point(-1, 1), cv::Point(0, 1), cv::Point(1, 1) };
    
    int dx = 0, dy = 0;
    double g = 0;
    for (int y = 0; y < graySrc.rows; y++)
    {
        for (int x = 0; x < graySrc.cols; x++)
        {
            if (x < 1 || y < 1 || x > graySrc.cols-2 || y > graySrc.rows-2) continue;
            
            cv::Point p(x, y);
            cv::Point xp[] = { p+aroundx[0], p+aroundx[1], p+aroundx[2], p+aroundx[3], p+aroundx[4], p+aroundx[5] };
            cv::Point yp[] = { p+aroundy[0], p+aroundy[1], p+aroundy[2], p+aroundy[3], p+aroundy[4], p+aroundy[5] };
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

void Mycv::decreaseColorsWithKmeans(const Mat& src, Mat& dst)
{
    const int cluster_count = 8; /* number of cluster */
    
    Mat points;
    src.convertTo(points, CV_32FC3);
    points = points.reshape(3, src.rows*src.cols);
    
    Mat_<int> clusters(points.size(), CV_32SC1);
    Mat centers;
    kmeans(points, cluster_count, clusters,
           cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0), 1, KMEANS_PP_CENTERS, centers);
    
    dst = Mat(src.size(), src.type());
    MatIterator_<Vec3f> itf = centers.begin<Vec3f>();
    MatIterator_<Vec3b> itd = dst.begin<Vec3b>(), itd_end = dst.end<Vec3b>();
    
    long r = 0, g = 0, b = 0;
    for (int i = 0; i < centers.rows; i++) {
        for (int j = 0; j < clusters.rows; j++) {
            if (i == clusters(1,j)) {
                r += points.at<int>(0, j);
                g += points.at<int>(1, j);
                b += points.at<int>(2, j);
            }
        }
    }
    
    for(int i=0; itd != itd_end; ++itd, ++i) {
        Vec3f color = itf[clusters(1,i)];
        (*itd)[0] = saturate_cast<uchar>(color[1]);
        (*itd)[1] = saturate_cast<uchar>(color[1]);
        (*itd)[2] = saturate_cast<uchar>(color[1]);
    }
    
    namedWindow( "src", CV_WINDOW_AUTOSIZE );
    imshow( "src", src);
    Draw::draw(dst);
}


void Mycv::SWT(const Mat& edge, Mat& swt)
{
    Mat gray;
    grayscale(srcImage, gray);
    Mat_<double> gradient = Mat_<double>(gray.rows, gray.cols);
    sobelFiltering(gray, gradient);
    
    swt = Mat_<double>::zeros(gray.rows, gray.cols);
    swt = MAXFLOAT;
    int ecount = 0, fcount = 0;
    double max = 0;
    Mat_<int> nmap = Mat_<int>::zeros(swt.rows, swt.cols);
    
    for (int y = 0; y < edge.rows; y++) {
        for (int x = 0; x < edge.cols; x++) {
            
            if (x < 1 || y < 1 || x > edge.cols-2 || y > edge.rows-2) continue;
            
            int e = edge.at<uchar>(y, x);
            if (e == 0) continue;
            
            Point_<double> p(x, y);
            double prad = gradient.at<double>(y, x);
            Point_<double> ray(-cos(prad), sin(prad));
            Point_<int> q, diff;
            bool findflag = false;
            int e1 = 0, e2 = 0, e3 = 0;
            
            for (double n = 1; n < 300; n++) {
                
                // Compute a ray point
                q = p - n * ray;
                if (q.x < 1 || q.y < 1 || q.x > edge.cols-2 || q.y > edge.rows-2) continue;
                
                // Check whether the ray point is an edge point.
                e1 = edge.at<uchar>(q.y-1, q.x);
                e2 = edge.at<uchar>(q.y, q.x);
                e3 = edge.at<uchar>(q.y+1, q.x);
                if ((abs(p.x - q.x)>1 || abs(p.y - q.y)>1) &&
                    (e1 > 0 || e2 > 0 || e3 > 0))
                {
                    // Compute distance between p and r.
                    diff = Point(p.x-q.x, p.y-q.y);
                    double distance = sqrt(diff.x*diff.x + diff.y*diff.y);
                    
                    // If gradients of p and r are opposite
                    double rrad = gradient.at<double>(q.y, q.x);
                    if (fabs(prad - rrad) > M_PI_4_5) {
                        
                        // Write the distance in p and r points on swt mat.
                        if (swt.at<double>(p.y, p.x) > distance)
                            swt.at<double>(p.y, p.x) = distance;
                        if (swt.at<double>(q.y, q.x) > distance)
                            swt.at<double>(q.y, q.x) = distance;
                        
                        // Write the distance in points on the ray
                        for (int temp = 1; temp < n; temp++) {
                            Point r = p - temp * ray;
                            if (r.x < 1 || r.y < 1 || r.x > edge.cols-2 || r.y > edge.rows-2)
                                continue;
                            if (swt.at<double>(r.y, r.x) > distance)
                                swt.at<double>(r.y, r.x) = distance;
                        }
                        
                        // Compute a max distance.
                        if (max < distance) max = distance;
                        // Check the findflag.
                        findflag = true;
                        // Write n into nmap.
                        nmap.at<int>(y, x) = n;
                        
                        break;
                    } else {
                        // Write n into nmap.
                        nmap.at<int>(y, x) = n;
                        break;
                    }
                }
            }
            
            ecount++;
            if (findflag) fcount++;
        }
    }
    
    // Compute medians.
    for (int y = 0; y < edge.rows; y++) {
        for (int x = 0; x < edge.cols; x++) {
            
            if (x < 1 || y < 1 || x > edge.cols-2 || y > edge.rows-2) continue;
            
            int e = edge.at<uchar>(y, x);
            if (e == 0) continue;
            
            Point_<double> p(x, y);
            double prad = gradient.at<double>(y, x);
            Point_<double> ray(-cos(prad), sin(prad));
            Point r;
            vector<Point> rayPoints;
            vector<double> distances;
            double sw, median;
            
            for (int n = 1; n < nmap.at<int>(y, x); n++) {
                r = p - n * ray;
                if (r.x < 1 || r.y < 1 || r.x > edge.cols-2 || r.y > edge.rows-2)
                    continue;
                sw = swt.at<double>(r.y, r.x);
                if (sw < MAXFLOAT) {
                    rayPoints.push_back(Point(r));
                    distances.push_back(sw);
                }
            }
            
            if (distances.size() == 0) continue;
            median = distances[distances.size()/2];
            for (int i = 0; i < rayPoints.size(); i++) {
                swt.at<double>(rayPoints[i].y, rayPoints[i].x) = median;
            }
        }
    }

    cout << "count:" << fcount << "/" << ecount << ", " << edge.rows*edge.cols << endl;
    
    Draw::drawSWT(swt, max);
}

Mycv::~Mycv()
{
    
}

