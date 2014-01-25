
#include "ObjectDetector.h"

#pragma mark -
#pragma mark Thread Methods & Instances

typedef struct {
    Mycv mycv;
    Mat edge;
    Mat_<double> gradient;
} SWT_THREAD_ARG;

typedef struct {
    Object* object;
    int index;
    Mat_<int> table;
    Mat srcImage;
    int type;
    
} CORR_THREAD_ARG;

// SWT Instances
pthread_mutex_t mutex;
vector<SWTObject> swtobjects;
void* swt_minus_thread(void* pParam);
void* swt_plus_thread(void* pParam);
Mat swtm, swtp;
vector<vector<cv::Point> > compm, compp;

pthread_mutex_t corr_mutex;
void* find_corr_pairs_thread(void* param);


#pragma mark -
#pragma mark Inline Methods

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



#pragma mark -
#pragma mark Constructors

ObjectDetector::ObjectDetector()
{
    
}

ObjectDetector::ObjectDetector(const string& path)
{
    filepath = path;
    filename = filepath.substr(filepath.find_last_of("/")+1);
    
    srcImage = imread(filepath, CV_LOAD_IMAGE_COLOR);
    srcIplImage = srcImage;
    
    srcW = srcImage.cols;
    srcH = srcImage.rows;
    
    id_count = 0;
}

ObjectDetector::~ObjectDetector()
{
    
}


#pragma mark -
#pragma mark Interface Methods


void ObjectDetector::detect(vector<Object*>& objects)
{
    Mycv mycv(srcImage);
    
    // 各色でグレースケール＆エッジ抽出
    Mat rgray, ggray, bgray, redge, gedge, bedge, edge;
//    mycv.grayscale(srcImage, rgray, MYCV_GRAY_R);
    mycv.grayscale(srcImage, ggray, MYCV_GRAY_G);
//    mycv.grayscale(srcImage, bgray, MYCV_GRAY_B);
//    mycv.canny(rgray, redge);
    mycv.canny(ggray, gedge);
//    mycv.canny(bgray, bedge);
//    mycv.mergeEdges(redge, gedge, bedge, edge);
    edge = gedge;
    
    // 輪郭抽出
    cv::vector<cv::Vec4i> hierarchy;
    cv::vector<cv::vector<cv::Point> > contours;
    mycv.contours(edge, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    
    // 輪郭から Object 作成
    createObjects(contours, objects);
    cout << objects.size() << " objects are created from contours." << endl;
    
    // Unsharp Masking からの MSER 抽出
    Mat imgUnsharp;
    mycv.unsharpMasking(srcImage, imgUnsharp, 1);
    vector<MSERegion> msers;
    mycv.MSERs(imgUnsharp, msers);
    
    // MSER と包含関係を使って補正
    mergeApartContours(objects, msers);
    if (objects.size() == 0) return;
    mergeIncludedObjects(objects);
    
    // SWT と Color 類似度を使って補正
    Mat_<double> gradient = Mat_<double>(srcImage.rows, srcImage.cols);
    mycv.sobelFiltering(ggray, gradient);
    
    // Object の勾配方向計算
    gradientOfObjects(objects, gradient);
    
    // Corresponding Pair の探索 と 特徴量計算
    findCorrPairs(objects, gradient);
    gradientOfCorrPairs(objects, gradient);
    
    // 特徴量計算
    setFeatures(objects);           // 特徴量をセット
    
    Draw::draw(Draw::drawInnerAreaOfObjects(srcImage, objects)); // Echar描画
    
}


#pragma mark -
#pragma mark Assistant Methods

// Create Objects and Set Contours
void ObjectDetector::createObjects(const vector<vector<cv::Point> >& contours, vector<Object*>& objects)
{
    int imgArea = srcImage.rows*srcImage.cols;
    double ratio = 0;
    for (int i = 0; i < contours.size(); i++) {
        
        // ID生成
        id_count++;
        stringstream Id;
        Id << filename << id_count;
        
        // Object生成
        Object *obj = new Object(Id.str(), filepath, contours[i], cv::Size(srcImage.cols, srcImage.rows));
        ratio = (double)obj->contourArea / obj->rectArea;
        
        if (obj->rectArea>30 && obj->rectArea < imgArea*0.25f &&
            ratio > 0.4f && obj->aspectRatio < 5)
        {
            objects.push_back(obj);
        }
    }
}

// Compare MBR between Contours and MSER
void ObjectDetector::mergeApartContours(vector<Object*>& objects, vector<MSERegion>& msers)
{
    cv::Rect objRect, mserRect, interRect;
    double objRectRatio = 0, mserRectRatio = 0;
    double objRatioThre = 0.5f, mserThreshold = 0.3f;
    
    // ObjectとMSERの矩形の重なり割合でObjectが含むMSERを取得する
    for (int i = 0; i < objects.size(); i++)
    {
        objRect = objects[i]->rect;
        objRectRatio = 0;
        
        for (int j = 0; j < msers.size(); j++)
        {
            mserRect = msers[j].rect;
            interRect = *intersect(objects[i]->rect, msers[j].rect);
            
            objRectRatio = (double)interRect.area() / objRect.area();
            mserRectRatio = (double)interRect.area() / mserRect.area();
            
            if (objRectRatio > objRatioThre)
            {
                if (abs(objRectRatio - 1) < abs(objects[i]->rectRatio - 1)
                    && mserRectRatio > mserThreshold)
                {
                    objects[i]->rectRatio = objRectRatio;
                    objects[i]->mserIndex = j;
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
            Object *baseObj = objects[baseIndex];
            Object *tempObj = objects[tempIndex];
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
void ObjectDetector::mergeIncludedObjects(vector<Object*>& objects)
{
    sort(objects.begin(), objects.end(), Object::isLeftLarge);
    
    
    cv::Rect largeRect, smallRect, interRect;
    double lratio = 0;
    vector<int> removeIndexes;
    
    for (int i = 0; i < objects.size()-1; i++)
    {
        largeRect = objects[i]->rect;
        for (int j = i+1; j < objects.size(); j++)
        {
            smallRect = objects[j]->rect;
            interRect = largeRect & smallRect;
            if (interRect.area() == 0) continue;
            
            lratio = objects[i]->longLength / objects[j]->longLength;
            bool contains = interRect.area() == smallRect.area();
            bool enoughSmall = lratio < 4.0;
            
            if (contains && enoughSmall) //wratio < 4 || hratio < 4)
            {
                objects[i]->children.push_back(j);
                removeIndexes.push_back(j);
            }
        }
        
        int index = 0;
        for (int j = 0; j < objects[i]->children.size(); j++)
        {
            index = objects[i]->children[j];
            objects[i]->mergeObject(*objects[index]);
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

// Create Objects from SWT Matrix and components
void ObjectDetector::createSWTObjects(vector<SWTObject>& swtobjects, const Mat& swt, vector<vector<cv::Point> >& components)
{
    for (int i = 0; i < components.size(); i++) {
        
        long size = components[i].size();
        if (size < 4) continue;
        
        SWTObject swtobj(components[i], swt);
        
        if (swtobj.aspectRatio >= 15) continue;
        if (swtobj.variance >= pow(swtobj.mean, 2)) continue;
        if (MAX(swtobj.width, swtobj.height) > 300) continue;
        if (swtobj.region.size() < 10) continue;
        if (swtobj.areaRatio < 0.2f) continue;
        
        // 平均色の計算
        swtobj.color = computeColor(srcImage, components[i]);
        
        swtobjects.push_back(swtobj);
    }
}

inline Scalar ObjectDetector::computeColor(const Mat& srcImage, const vector<cv::Point>& region)
{
    Scalar color = Scalar(0, 0, 0);
    Vec3b temp;
    long size = region.size();
    for (int j = 0; j < size; j++) {
        temp = srcImage.at<Vec3b>(region[j].y, region[j].x);
        color[0] += temp[0];
        color[1] += temp[1];
        color[2] += temp[2];
    }
    if (size > 0) {
        color[0] /= size;
        color[1] /= size;
        color[2] /= size;
    }
    return color;
}

// Compute Gradients of Objects
void ObjectDetector::gradientOfObjects(vector<Object*>& objects, const Mat_<double>& gradients)
{
    cv::Point p;
    double t = 0;
    for (int i = 0; i < objects.size(); i++)
    {
        vector<double> *thetas = new vector<double>();
        for (int j = 0; j < objects[i]->contourPixels.size(); j++)
        {
            p = objects[i]->contourPixels[j];
            t = gradients.at<double>(p.y, p.x);
            thetas->push_back(t);
        }
        objects[i]->thetas = *thetas;
    }
}

// Decide the gradient direction
bool ObjectDetector::isPositiveDirection(Object* object)
{
    long posi = 0, nega = 0, t = 0, l = 0, r = 0, b = 0;
    double theta = 0;
    cv::Point p, tl, br;
    
    tl = object->rect.tl();
    br = object->rect.br();
    vector<cv::Point> *tempsurr = new vector<cv::Point>();
    vector<double> *tempThetas = new vector<double>();
    
    for (int j = 0; j < object->contourPixels.size(); j++)
    {
        p = object->contourPixels[j];
        
        if (p.y == tl.y) // TOP
        {
            theta = object->thetas[j];
            if (theta >= 0) nega++;
            else posi++;
            t++;
            tempsurr->push_back(p);
            tempThetas->push_back(theta);
        }
        else if (p.x == tl.x) // LEFT
        {
            theta = object->thetas[j];
            if (fabs(theta) >= M_PI_2) nega++;
            else posi++;
            l++;
            tempsurr->push_back(p);
            tempThetas->push_back(theta);
        }
        else if (p.x == br.x-1) //RIGHT
        {
            theta = object->thetas[j];
            if (fabs(theta) >= M_PI_2) posi++;
            else nega++;
            r++;
            tempsurr->push_back(p);
            tempThetas->push_back(theta);
        }
        else if (p.y == br.y-1) // BOTTOM
        {
            theta = object->thetas[j];
            if (theta >= 0) posi++;
            else nega++;
            b++;
            tempsurr->push_back(p);
            tempThetas->push_back(theta);
        }
    }
    
    object->surroundings = *tempsurr;
    object->surrThetas = *tempThetas;
    
    if (posi >= nega) return true;
    else return false;
}

Scalar ObjectDetector::getColor(int x, int y)
{
    unsigned char r, g, b;
    
    int index = srcIplImage.widthStep * y + srcIplImage.nChannels * x;
    b = srcIplImage.imageData[index + 0];
    g = srcIplImage.imageData[index + 1];
    r = srcIplImage.imageData[index + 2];
    
    return Scalar(r, g, b);
}

// Find Corresponding Pairs & Compute bounding circle
void ObjectDetector::findCorrPairs(vector<Object*>& objects, const Mat& gradients)
{
    Mat_<int> table = createImageTable(objects);
    int minusType = -1, plusType = 1;
    
    for (int i = 0; i < objects.size(); i++)
    {
        findPairs(objects[i], i, minusType, table);
        findPairs(objects[i], i, plusType, table);
    }
}

void ObjectDetector::findPairs(Object*& object, int index, int type, Mat_<int> table)
{
    cv::Size size(object->width, object->height);
    cv::Point origin = object->origin;
    vector<double> thetas = object->thetas;
    vector<cv::Point> pixels = object->contourPixels;
    
    vector<cv::Point> corrPixels = vector<cv::Point>(pixels.begin(), pixels.end());
    vector<cv::Point> innerPixels = vector<cv::Point>();
    
    Point_<double> p, ray;
    Point_<int> q;
    cv::Point p1, p2, vp1(1,0), vp2(-1,0), hp1(0,1), hp2(0,-1);
    cv::Point innerp;
    
    bool findFlag = false;
    int count = 0;
    
    for (int j = 0; j < pixels.size(); j++)
    {
        // Initialize
        findFlag = false;
        innerPixels.clear();
        
        // p and ray
        p = pixels[j];
        ray = Point_<double>(-cos(thetas[j]), sin(thetas[j]));
        if (abs(ray.x) > abs(ray.y)) {
            p1 = hp1;
            p2 = hp2;
        } else {
            p1 = vp1;
            p2 = vp2;
        }
        
        for (int n = 2; n < 300; n++) {
            
            // Compute a ray point
            if (type>0) q = p + n * ray;
            else q = p - n * ray;
            
            if (!isFullIn(srcW, srcH, q.x, q.y))
                continue;
            
            // Input q as an inner pixel
            innerPixels.push_back(Point(q.x, q.y));
            
            // Search the object table
            if (table.at<int>(q) == index) {
                corrPixels[j] = Point(q.x, q.y);
                findFlag = true;
                break;
            } else if (table.at<int>(q+p1) == index) {
                corrPixels[j] = Point((q+p1).x, (q+p1).y);
                findFlag = true;
                break;
            } else if (table.at<int>(q+p2) == index) {
                corrPixels[j] = Point((q+p2).x, (q+p2).y);
                findFlag = true;
                break;
            }
            
        }
        
        if (findFlag == false) {
            corrPixels[j] = Point(-1, -1);
            count++;
        } else {
            for (int k = 0; k < innerPixels.size(); k++) {
                innerp = innerPixels[k] - origin;
                if (isFullIn(object->width, object->height, innerp.x, innerp.y)) {
                    object->innerAreaMap.at<int>(innerp) = type;
                }
            }
        }
    }
    
    if (type>0) object->pCorrPairPixels = *new vector<Point>(corrPixels);
    else object->mCorrPairPixels = *new vector<Point>(corrPixels);
}

Mat_<int> ObjectDetector::createImageTable(const vector<Object*>& objects)
{
    int H = srcImage.rows, W = srcImage.cols;
    Mat_<int> table(H, W);
    table = -1;
    
    cv::Point p;
    for (int i = 0; i < objects.size(); i++) {
        for (int j = 0; j < objects[i]->contourPixels.size(); j++) {
            p = objects[i]->contourPixels[j];
            table.at<int>(p) = i;
        }
    }
    
    return table;
}

void ObjectDetector::gradientOfCorrPairs(vector<Object*>& objects, const Mat_<double>& gradients)
{
    cv::Point p;
    double t = 0;
    for (int i = 0; i < objects.size(); i++)
    {
        vector<double> *mthetas = new vector<double>();
        for (int j = 0; j < objects[i]->mCorrPairPixels.size(); j++)
        {
            p = objects[i]->mCorrPairPixels[j];
            if (p.x < 0 || p.y < 0) {
                mthetas->push_back(10);
            } else {
                t = gradients.at<double>(p.y, p.x);
                mthetas->push_back(t);
            }
        }
        objects[i]->mCorrThetas = *mthetas;
        
        vector<double> *pthetas = new vector<double>();
        for (int j = 0; j < objects[i]->mCorrPairPixels.size(); j++)
        {
            p = objects[i]->mCorrPairPixels[j];
            if (p.x < 0 || p.y < 0) {
                pthetas->push_back(10);
            } else {
                t = gradients.at<double>(p.y, p.x);
                pthetas->push_back(t);
            }
        }
        objects[i]->pCorrThetas = *pthetas;
    }
}

// Set Features to the Object
void ObjectDetector::setFeatures(vector<Object*>& objects)
{
    for (int i = 0; i < objects.size(); i++) {
        
        // Compute features
        objects[i]->computeFeatures(srcImage);
        
        
        vector<double> features;
        
        // Echar
        /* 0 */ features.push_back(objects[i]->Gangle / M_PI);
        /* 1 */ features.push_back(objects[i]->Fcorr);
        /* 2 */ features.push_back(objects[i]->Echar);
        
        // Color
        /* 3 */ features.push_back((double)objects[i]->color[0]);
        /* 4 */ features.push_back((double)objects[i]->color[1]);
        /* 5 */ features.push_back((double)objects[i]->color[2]);
        /* 6 */ features.push_back((double)objects[i]->labcolor[0]);
        /* 7 */ features.push_back((double)objects[i]->labcolor[1]);
        /* 8 */ features.push_back((double)objects[i]->labcolor[2]);
        
        // Stroke width
        /* 9 */ features.push_back(objects[i]->strokeWidth);
        /* 10 */ features.push_back(objects[i]->varStrokeWidth);
        
        // TODO: Contour roughness
        
        // Rect ratio
        /* 11 */ features.push_back(objects[i]->rectRatio);
        
        // Aspect ratio
        /* 12 */ features.push_back(objects[i]->aspectRatio);
        
        // Long length
        /* 13 */ features.push_back(objects[i]->longLength);
        
        // Area ratio
        /* 14 */ features.push_back(objects[i]->areaRatio);
        
        // Set features
        objects[i]->features = features;
    }
}

cv::Rect* ObjectDetector::intersect(const cv::Rect& rect1, const cv::Rect& rect2)
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

void* swt_minus_thread(void* param)
{
    SWT_THREAD_ARG *arg =(SWT_THREAD_ARG*)param;
    
    Mycv mycv = arg->mycv;
    
    clock_t swtm_clock = mycv.SWTMinus(arg->edge, arg->gradient, swtm);
    cout << "SWT Minus Time: " << swtm_clock << "ms" << endl;
    clock_t swtcompm_clock = mycv.SWTComponents(swtm, compm);
    cout << "SWT Minus Components Time: " << swtcompm_clock << "ms" << endl;
    
    return NULL;
}

void* swt_plus_thread(void* param)
{
    SWT_THREAD_ARG *arg =(SWT_THREAD_ARG*)param;
    
    Mycv mycv = arg->mycv;
    
    clock_t swtp_clock = mycv.SWTPlus(arg->edge, arg->gradient, swtp);
    cout << "SWT Plus Time: " << swtp_clock << "ms" << endl;
    clock_t swtcompp_clock = mycv.SWTComponents(swtp, compp);
    cout << "SWT Plus Components Time: " << swtcompp_clock << "ms" << endl;
    
    return NULL;
}

void* find_corr_pairs_thread(void* param)
{
    CORR_THREAD_ARG *arg = (CORR_THREAD_ARG*)param;
    Object *object = arg->object;
    int index = arg->index;
    Mat_<int> table(arg->table);
    int type = arg->type;
    Mat srcImage(arg->srcImage);
    
    // Object情報取得（変数ロック）
    pthread_mutex_lock(&corr_mutex);
    cv::Size size(object->width, object->height);
    cv::Point origin = object->origin;
    vector<double> *thetas = &(object->thetas);
    vector<cv::Point> *pixels = &(object->contourPixels);
    pthread_mutex_unlock(&corr_mutex);
    
    vector<cv::Point> corrPixels = vector<cv::Point>(pixels->begin(), pixels->end());
    vector<cv::Point> innerPixels = vector<cv::Point>();
    innerPixels.reserve(size.width*size.height);
    vector<cv::Point> tempInnerPixels;
    tempInnerPixels.reserve(innerPixels.size());
    
    Point_<double> p, ray;
    Point_<int> q;
    cv::Point p1, p2, vp1(1,0), vp2(-1,0), hp1(0,1), hp2(0,-1);
    cv::Point innerp;
    
    bool findFlag = false;
    int count = 0;
    
    for (int j = 0; j < pixels->size(); j++)
    {
        // Initialize
        findFlag = false;
        tempInnerPixels.clear();
        
        // p and ray
        p = (*pixels)[j];
        ray = Point_<double>(-cos((*thetas)[j]), sin((*thetas)[j]));
        if (abs(ray.x) > abs(ray.y)) {
            p1 = hp1;
            p2 = hp2;
        } else {
            p1 = vp1;
            p2 = vp2;
        }
        
        for (int n = 2; n < 300; n++) {
            
            // Compute a ray point
            if (type>0) q = p + n * ray;
            else q = p - n * ray;
            
            if (!isFullIn(srcImage.cols, srcImage.rows, q.x, q.y))
                continue;

            // Input q as an inner pixel
            tempInnerPixels.push_back(Point(q.x, q.y));
            
            // Search the object table
            if (table.at<int>(q) == index) {
                corrPixels[j] = Point(q.x, q.y);
                findFlag = true;
            } else if (table.at<int>(q+p1) == index) {
                corrPixels[j] = Point((q+p1).x, (q+p1).y);
                findFlag = true;
            } else if (table.at<int>(q+p2) == index) {
                corrPixels[j] = Point((q+p2).x, (q+p2).y);
                findFlag = true;
            }
            
        }
        
        if (findFlag == false) {
            corrPixels[j] = Point(-1, -1);
            count++;
        } else {
            for (int k = 0; k < tempInnerPixels.size(); k++) {
                innerPixels.push_back(Point(tempInnerPixels[k]));
            }
        }
    }
    
    pthread_mutex_lock(&corr_mutex);
    object->corrPairPixels = *new vector<Point>(corrPixels);
    pthread_mutex_unlock(&corr_mutex);
    
    return NULL;
}

