
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



#pragma mark -
#pragma mark SWT Assistant Methods

inline bool isIn(int w,int h,int x,int y)
{
    return 0<=x && x<w && 0<=y && y<h;
}

inline bool isFullIn(int w,int h,int x,int y)
{
    return 0<x && x<w-1 && 0<y && y<h-1;
}

inline int getIntAt(const Mat& matrix,int x,int y)
{
    return matrix.at<uchar>(y, x);
}

inline double getDoubleAt(const Mat& matrix,int x,int y)
{
    return matrix.at<double>(y, x);
}

inline bool isSimilar(double a, double b) {
    return MAX(a, b) / MIN(a, b) < 4.0f;
}

//aの属すグループの代表に向かって経路圧縮（代表を返す）
inline int compress(vector<int>& parents,int a)
{
    while(a!=parents[a])
    {
        parents[a]=parents[parents[a]];
        a=parents[a];
    }
    return a;
}

//aの属すグループとbの属すグループを併合（併合後の代表を返す）
inline int link(vector<int>& parents,int a,int b)
{
    a=compress(parents,a);
    b=compress(parents,b);
    if(a<b)
        return parents[b]=a;
    else
        return parents[a]=b;
}

//番号とびとびなラベルを0,1,2,...に貼り替え
inline int relabel(vector<int>& parents)
{
    int index=0;
    for(int k=0;k<(int)parents.size();k++)
    {
        if(k==parents[k])
            parents[k]=index++;
        else
            parents[k]=parents[parents[k]];
    }
    return index;
}

#pragma mark -
#pragma mark SWT Methods


void Mycv::SWT(const Mat& edge, const Mat& gradient, Mat& swt)
{
    bool findflag = false;
    const int H = edge.rows, W = edge.cols;
    int e1 = 0, e2 = 0, e3 = 0, ecount = 0, fcount = 0;
    double pradian = 0, rradian = 0, distance = 0, max = 0;
    Point_<double> p, ray;
    Point_<int> q, diff, r;
    
    swt = Mat_<double>::zeros(H, W);
    swt = MAXFLOAT;
    Mat_<int> nmap = Mat_<int>::zeros(H, W);
    
    for (int y = 0; y < H; y++) 
        for (int x = 0; x < W; x++)
        {
            if (!isFullIn(W, H, x, y)) continue;
            
            int e = getIntAt(edge, x, y);
            if (e == 0) continue;
            
            findflag = false;
            pradian = getDoubleAt(gradient, x, y);
            p = Point_<double>(x, y);
            ray = Point_<double>(-cos(pradian), sin(pradian));
            
            for (double n = 1; n < 300; n++) {
                
                // Compute a ray point
                q = p - n * ray;
                if (!isFullIn(W, H, q.x, q.y)) continue;
                
                // Check whether the ray point is an edge point.
                e1 = getIntAt(edge, q.x, q.y-1);
                e2 = getIntAt(edge, q.x, q.y  );
                e3 = getIntAt(edge, q.x, q.y+1);
                if ((abs(p.x - q.x)>1 || abs(p.y - q.y)>1) &&
                    (e1 > 0 || e2 > 0 || e3 > 0))
                {
                    // Compute the distance between p and r.
                    diff = Point(p.x-q.x, p.y-q.y);
                    distance = sqrt(diff.x*diff.x + diff.y*diff.y);
                    
                    // If gradients of p and r are opposite
                    rradian = getDoubleAt(gradient, q.x, q.y);
                    if (fabs(pradian - rradian) > M_PI_2_3) {
                        
                        // Write the distance in p and r points on swt mat.
                        if (getDoubleAt(swt, p.x, p.y) > distance)
                            swt.at<double>(p.y, p.x) = distance;
                        if (getDoubleAt(swt, q.x, q.y) > distance)
                            swt.at<double>(q.y, q.x) = distance;
                        
                        // Write the distance in points on the ray
                        for (int temp = 1; temp < n; temp++) {
                            r = p - temp * ray;
                            if (!isFullIn(W, H, r.x, r.y)) continue;
                            if (getDoubleAt(swt, r.x, r.y) > distance)
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
    
    double sw = 0, median = 0;
    vector<Point> rayPoints;
    vector<double> distances;
    
    // Compute medians.
    for (int y = 0; y < H; y++) 
        for (int x = 0; x < W; x++)
        {
            if (!isFullIn(W, H, x, y)) continue;
            
            int e = getIntAt(edge, x, y);
            if (e == 0) continue;
            
            pradian = getDoubleAt(gradient, x, y);
            p = Point_<double>(x, y);
            ray = Point_<double>(-cos(pradian), sin(pradian));
            
            for (int n = 1; n < getIntAt(nmap, x, y); n++) {
                r = p - n * ray;
                if (!isFullIn(W, H, r.x, r.y)) continue;
                
                sw = getDoubleAt(swt, r.x, r.y);
                if (sw < MAXFLOAT) {
                    rayPoints.push_back(Point(r));
                    distances.push_back(sw);
                }
            }
            
            if (distances.size() == 0) continue;
            sort(distances.begin(), distances.end());
            median = distances[distances.size()/2];
            for (int i = 0; i < rayPoints.size(); i++) {
                swt.at<double>(rayPoints[i].y, rayPoints[i].x) = median;
            }
        }
}


void Mycv::SWTComponents(const Mat& swt, vector<vector<cv::Point> >& components )
{
    // 初期化
    const int W=swt.cols;
    const int H=swt.rows;
    Mat_<int> label = Mat_<int>::zeros(H, W);
    
    // 関数を複数回呼び出すときメモリ確保が何度も生じるので、
    // 例えばリアルタイム処理では関数外で確保するのがベター。
    vector<int> parents;
    parents.reserve(BUF_LABEL);
    
    int index=0;
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++)
        {
            // 隣接画素（４近傍）との連結チェック
            double c = getDoubleAt(swt,x,y);
            bool flagA=(isIn(W,H,x-1,y  ) && isSimilar(c, getDoubleAt(swt,x-1,y  ))); //左
            bool flagB=(isIn(W,H,x  ,y-1) && isSimilar(c, getDoubleAt(swt,x  ,y-1))); //上
            bool flagC=(isIn(W,H,x-1,y-1) && isSimilar(c, getDoubleAt(swt,x-1,y-1))); //左上
            bool flagD=(isIn(W,H,x+1,y-1) && isSimilar(c, getDoubleAt(swt,x+1,y-1))); //右上
            
            // 着目画素と連結画素を併合
            label(y,x)=index;
            if((flagA|flagB|flagC|flagD)==true)
            {
                parents.push_back(index);
                if(flagA) label(y,x)=link(parents,label(y,x),label(y  ,x-1));
                if(flagB) label(y,x)=link(parents,label(y,x),label(y-1,x  ));
                if(flagC) label(y,x)=link(parents,label(y,x),label(y-1,x-1));
                if(flagD) label(y,x)=link(parents,label(y,x),label(y-1,x+1));
                parents.pop_back();
            }
            else
                parents.push_back(index++);
        }
    
    // 再ラベリング
    int regions=relabel(parents);
    components.reserve(regions);
    for (int i = 0; i < regions; i++) components.push_back(vector<Point>());
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++)
        {
            int lbl = parents[label(y,x)];
            label(y,x) = lbl;
            components[lbl].push_back(Point(x, y));
        }

}


Mycv::~Mycv()
{
    
}

