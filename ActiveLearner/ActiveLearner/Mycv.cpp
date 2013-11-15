
#include "Mycv.h"

vector<string> Mycv::split(const string& str, char delim)
{
    istringstream iss(str); string tmp; vector<string> res;
    while(getline(iss, tmp, delim)) res.push_back(tmp);
    return res;
}

#pragma mark -
#pragma mark Constructor

Mycv::Mycv(const string& filepath)
{
    srcImage = imread(filepath, CV_LOAD_IMAGE_COLOR);
    srcIplImage = srcImage;
    
    vector<string> comps = split(filepath, '/');
    filename = comps[comps.size()-1];
    id_count = 0;
}

#pragma mark -
#pragma mark Interface Methods

void Mycv::detector(vector<Object>& objects)
{
    // Detect contours
    Mat imgGray, imgCanny;
    Mycv::grayscale(srcImage, imgGray);
    Mycv::canny(imgGray, imgCanny);
    
    // Draw::draw(imgCanny);
    
    cv::vector<cv::Vec4i> hierarchy;
    cv::vector<cv::vector<cv::Point> > contours;
    Mycv::contours(imgCanny, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    
    
    // Create objects
    Mycv::createObjects(contours, objects);
    cout << "create objects:" << objects.size() << endl;
    
    // Detect MSERs
    Mat imgUnsharp;
    Mycv::unsharpMasking(srcImage, imgUnsharp, 1);
    
    vector<MSERegion> msers;
    Mycv::MSERs(imgUnsharp, msers);
    
    // Interpolate contours with MSERs and Inclusion Relationship
    Mycv::mergeApartContours(objects, msers);
    cout << "merge apart contours:" << objects.size() << endl;
    if (objects.size() == 0) return;
    Mycv::mergeIncludedObjects(objects);
    
    // Draw::drawObjects(srcImage, objects); // オブジェクト描画
    
    // Compute gradients
    Mat_<double> gradients = Mat_<double>(srcImage.rows, srcImage.cols);
    Mycv::sobelFiltering(imgGray, gradients);
    Mycv::gradientOfObjects(objects, gradients);
    
    // Draw::drawGradients(objects, gradients); // 勾配方向描画
    
    // Find corresponding pairs
    Mycv::findCorrPairs(objects, gradients);
    Mycv::gradientOfCorrPairs(objects, gradients);
    
    // Compute edge gradient features
    Mycv::computeEchar(objects);
    Mycv::computeStrokeWidth(objects);
    Mycv::setFeatures(objects);
    
    Draw::drawEchars(srcImage, objects); // Echar描画
    
    // Grouping objects
    vector<Text> texts = *new vector<Text>();
    Mycv::detectTexts(objects, texts);
    
    Draw::drawTexts(srcImage, texts);
}

void Mycv::detectTexts(vector<Object>& objects, vector<Text>& texts)
{
    // Sort
    sort(objects.begin(), objects.end(), Object::isLeftSmall);
    
    // Grouping アルゴリズム
    long length = objects.size();
    for (int i = 0; i < length; i++)
    {
        Object obj = objects[i];
        if (obj.grouped) continue;
        
        Text text = *new Text(/*obj*/);
        obj.grouped = true;
        vector<double> distance;
        double initd = 2 * distanceWithNearestNeighbor(obj, objects);
        distance.push_back(MIN(initd, obj.longLength));
        
        recursive_count = 0;
        groupingObjects(text, obj, distance, objects);
        
        if (text.aspectRatio >= 0.8 && text.aspectRatio <= 1.2)
        {
            distance.clear();
            distance.push_back(text.width);
            groupingObjects(text, obj, distance, objects);
        }
        
        texts.push_back(text);
    }
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
    int delta = 5, min_area = 0, max_area = 60000;
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


#pragma mark -
#pragma mark Feature Detection Processing Methods

// Create Objects and Set Contours
void Mycv::createObjects(const vector<vector<cv::Point> >& contours, vector<Object>& objects)
{
    int imgArea = srcImage.rows*srcImage.cols;
    double ratio = 0;
    for (int i = 0; i < contours.size(); i++) {
        
        // ID生成
        id_count++;
        stringstream Id;
        Id << filename << id_count;
        
        // Object生成
        Object obj = *new Object(Id.str(), filename, contours[i], cv::Size(srcImage.cols, srcImage.rows));
        ratio = (double)obj.contourArea / obj.rectArea;
        
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
void Mycv::mergeIncludedObjects(vector<Object>& objects)
{
    sort(objects.begin(), objects.end(), Object::isLeftLarge);
    
    cv::Rect largeRect, smallRect, interRect;
    double wratio = 0, hratio = 0, lratio = 0;
    vector<int> removeIndexes;
    
    cout << "merge included objects:" << objects.size() << endl;
    for (int i = 0; i < objects.size()-1; i++)
    {
        largeRect = objects[i].rect;
        for (int j = i+1; j < objects.size(); j++)
        {
            smallRect = objects[j].rect;
            interRect = *intersect(largeRect, smallRect);
            if (interRect.area() == 0) continue;
            
            wratio = (double)max(smallRect.width, largeRect.width) / min(smallRect.width, largeRect.width);
            hratio = (double)max(smallRect.height, largeRect.height) / min(smallRect.height, largeRect.height);
            lratio = objects[i].longLength / objects[j].longLength;
            
            if (lratio < 4)//wratio < 4 || hratio < 4)
            {
                objects[i].children.push_back(j);
                removeIndexes.push_back(j);
            }
        }
        
        int index = 0;
        for (int j = 0; j < objects[i].children.size(); j++)
        {
            index = objects[i].children[j];
            objects[i].mergeObject(objects[index]);
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

// Compute Gradients of Objects
void Mycv::gradientOfObjects(vector<Object>& objects, const Mat_<double>& gradients)
{
    cv::Point p;
    double t = 0;
    for (int i = 0; i < objects.size(); i++)
    {
        vector<double> *thetas = new vector<double>();
        for (int j = 0; j < objects[i].contourPixels.size(); j++)
        {
            p = objects[i].contourPixels[j];
            t = gradients.at<double>(p.y, p.x);
            thetas->push_back(t);
        }
        objects[i].thetas = *thetas;
    }
}

// Decide the gradient direction
bool Mycv::isPositiveDirection(Object& object)
{
    long posi = 0, nega = 0, t = 0, l = 0, r = 0, b = 0;
    double theta = 0;
    cv::Point p, tl, br;
    
    tl = object.rect.tl();
    br = object.rect.br();
    vector<cv::Point> *tempsurr = new vector<cv::Point>();
    vector<double> *tempThetas = new vector<double>();
    
    for (int j = 0; j < object.contourPixels.size(); j++)
    {
        p = object.contourPixels[j];
        
        if (p.y == tl.y) // TOP
        {
            theta = object.thetas[j];
            if (theta >= 0) nega++;
            else posi++;
            t++;
            tempsurr->push_back(p);
            tempThetas->push_back(theta);
        }
        else if (p.x == tl.x) // LEFT
        {
            theta = object.thetas[j];
            if (fabs(theta) >= M_PI_2) nega++;
            else posi++;
            l++;
            tempsurr->push_back(p);
            tempThetas->push_back(theta);
        }
        else if (p.x == br.x-1) //RIGHT
        {
            theta = object.thetas[j];
            if (fabs(theta) >= M_PI_2) posi++;
            else nega++;
            r++;
            tempsurr->push_back(p);
            tempThetas->push_back(theta);
        }
        else if (p.y == br.y-1) // BOTTOM
        {
            theta = object.thetas[j];
            if (theta >= 0) posi++;
            else nega++;
            b++;
            tempsurr->push_back(p);
            tempThetas->push_back(theta);
        }
    }
    
    object.surroundings = *tempsurr;
    object.surrThetas = *tempThetas;
    
    if (posi >= nega) return true;
    else return false;
}

Scalar Mycv::getColor(int x, int y)
{
    unsigned char r, g, b;
    
    int index = srcIplImage.widthStep * y + srcIplImage.nChannels * x;
    b = srcIplImage.imageData[index + 0];
    g = srcIplImage.imageData[index + 1];
    r = srcIplImage.imageData[index + 2];
    
    return Scalar(r, g, b);
}

// Find Corresponding Pairs
void Mycv::findCorrPairs(vector<Object>& objects, const Mat& gradients)
{
    int** table = createImageTable(objects);
    vector<double> thetas;
    vector<cv::Point> pixels, corrPixels;
    vector<int> candidates;
    
    vector<Scalar> colors;
    
    bool isPositive = false;
    double a = 0, b = 0;
    int X = 0, Y = 0, ty = 0, by = 0, lx = 0, rx = 0;
    cv::Point bp;
    
    for (int i = 0; i < objects.size(); i++)
    {
        isPositive = isPositiveDirection(objects[i]);
        pixels = objects[i].contourPixels;
        thetas = objects[i].thetas;
        corrPixels = *new vector<Point>(pixels.begin(), pixels.end());
        colors = *new vector<Scalar>();
        
        ty = objects[i].rect.tl().y;
        lx = objects[i].rect.tl().x;
        rx = objects[i].rect.br().x;
        by = objects[i].rect.br().y;
        
        for (int j = 0; j < pixels.size(); j++)
        {
            // Color保存用
            vector<Scalar> tmp_colors;
            
            // 直線式計算
            bp = pixels[j];
            a = - tan(thetas[j]);
            
            // 探索
            if (fabs(a) <= 1) {
                b = bp.y - a * bp.x;
                if (a >= 0) {   // a>=0
                    if (isPositive) {   // 勾配方向に探索
                        if (thetas[j] >= 0) {
                            for (int x = bp.x-2; x > lx; x--) {
                                
                                Y = (int)round(a * x + b);
                                if (Y < ty || Y > by) break;
                                
                                Scalar color = getColor(x, Y);
                                tmp_colors.push_back(color);
                                
                                if (x > lx && x < rx) {
                                    if (table[Y][x] == i) {
                                        corrPixels[j] = Point(x, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x-1] == i) {
                                        corrPixels[j] = Point(x-1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x+1] == i) {
                                        corrPixels[j] = Point(x+1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // xが減る方向
                        } else {
                            for (int x = bp.x+2; x < rx; x++) {
                                
                                Y = (int)round(a * x + b);
                                if (Y < ty || Y > by) break;
                                
                                Scalar color = getColor(x, Y);
                                tmp_colors.push_back(color);
                                
                                if (x > lx && x < rx) {
                                    if (table[Y][x] == i) {
                                        corrPixels[j] = Point(x, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x-1] == i) {
                                        corrPixels[j] = Point(x-1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x+1] == i) {
                                        corrPixels[j] = Point(x+1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // xが増える方向
                        }
                    } else {            // 勾配方向と逆に探索
                        if (thetas[j] >= 0) {
                            for (int x = bp.x+2; x < rx; x++) {
                                
                                Y = (int)round(a * x + b);
                                if (Y < ty || Y > by) break;
                                
                                Scalar color = getColor(x, Y);
                                tmp_colors.push_back(color);
                                
                                if (x > lx && x < rx) {
                                    if (table[Y][x] == i) {
                                        corrPixels[j] = Point(x, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x-1] == i) {
                                        corrPixels[j] = Point(x-1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x+1] == i) {
                                        corrPixels[j] = Point(x+1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // xが増える方向
                        } else {
                            for (int x = bp.x-2; x > lx; x--) {
                                
                                Y = (int)round(a * x + b);
                                if (Y < ty || Y > by) break;
                                
                                Scalar color = getColor(x, Y);
                                tmp_colors.push_back(color);
                                
                                if (x > lx && x < rx) {
                                    if (table[Y][x] == i) {
                                        corrPixels[j] = Point(x, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x-1] == i) {
                                        corrPixels[j] = Point(x-1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x+1] == i) {
                                        corrPixels[j] = Point(x+1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // xが減る方向
                        }
                    }
                } else {        // a<0
                    if (isPositive) {   // 勾配方向に探索
                        if (thetas[j] >= 0) {
                            for (int x = bp.x+2; x < rx; x++) {
                                
                                Y = (int)round(a * x + b);
                                if (Y < ty || Y > by) break;
                                
                                Scalar color = getColor(x, Y);
                                tmp_colors.push_back(color);
                                
                                if (x > lx && x < rx) {
                                    if (table[Y][x] == i) {
                                        corrPixels[j] = Point(x, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x-1] == i) {
                                        corrPixels[j] = Point(x-1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x+1] == i) {
                                        corrPixels[j] = Point(x+1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // xが増える方向
                        } else {
                            for (int x = bp.x-2; x > lx; x--) {
                                
                                Y = (int)round(a * x + b);
                                if (Y < ty || Y > by) break;
                                
                                Scalar color = getColor(x, Y);
                                tmp_colors.push_back(color);
                                
                                if (x > lx && x < rx) {
                                    if (table[Y][x] == i) {
                                        corrPixels[j] = Point(x, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x-1] == i) {
                                        corrPixels[j] = Point(x-1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x+1] == i) {
                                        corrPixels[j] = Point(x+1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // xが減る方向
                        }
                    } else {            // 勾配方向と逆に探索
                        if (thetas[j] >= 0) {
                            for (int x = bp.x-2; x > lx; x--) {
                                
                                Y = (int)round(a * x + b);
                                if (Y < ty || Y > by) break;
                                
                                Scalar color = getColor(x, Y);
                                tmp_colors.push_back(color);
                                
                                if (x > lx && x < rx) {
                                    if (table[Y][x] == i) {
                                        corrPixels[j] = Point(x, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x-1] == i) {
                                        corrPixels[j] = Point(x-1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x+1] == i) {
                                        corrPixels[j] = Point(x+1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // xが減る方向
                        } else {
                            for (int x = bp.x+2; x < rx; x++) {
                                
                                Y = (int)round(a * x + b);
                                if (Y < ty || Y > by) break;
                                
                                Scalar color = getColor(x, Y);
                                tmp_colors.push_back(color);
                                
                                if (x > lx && x < rx) {
                                    if (table[Y][x] == i) {
                                        corrPixels[j] = Point(x, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x-1] == i) {
                                        corrPixels[j] = Point(x-1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[Y][x+1] == i) {
                                        corrPixels[j] = Point(x+1, Y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // xが増える方向
                        }
                    }
                }
            } else {
                a = (double)1 / a;
                b = bp.x - a * bp.y;
                if (a >= 0) {   // a>=0
                    if (isPositive) {   // 勾配方向に探索
                        if (thetas[j] >= 0) {
                            for (int y = bp.y-2; y > ty; y--) {
                                
                                X = (int)round(a * y + b);
                                if (X < lx || X > rx) break;
                                
                                Scalar color = getColor(X, y);
                                tmp_colors.push_back(color);
                                
                                if (y > ty && y < by) {
                                    if (table[y][X] == i) {
                                        corrPixels[j] = Point(X, y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y-1][X] == i) {
                                        corrPixels[j] = Point(X, y-1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y+1][X] == i) {
                                        corrPixels[j] = Point(X, y+1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // yが減る方向
                        } else {
                            for (int y = bp.y+2; y < by; y++) {
                                
                                X = (int)round(a * y + b);
                                if (X < lx || X > rx) break;
                                
                                Scalar color = getColor(X, y);
                                tmp_colors.push_back(color);
                                
                                if (y > ty && y < by) {
                                    if (table[y][X] == i) {
                                        corrPixels[j] = Point(X, y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y-1][X] == i) {
                                        corrPixels[j] = Point(X, y-1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y+1][X] == i) {
                                        corrPixels[j] = Point(X, y+1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // yが増える方向
                        }
                    } else {            // 勾配方向と逆に探索
                        if (thetas[j] >= 0) {
                            for (int y = bp.y+2; y < by; y++) {
                                
                                X = (int)round(a * y + b);
                                if (X < lx || X > rx) break;
                                
                                Scalar color = getColor(X, y);
                                tmp_colors.push_back(color);
                                
                                if (y > ty && y < by) {
                                    if (table[y][X] == i) {
                                        corrPixels[j] = Point(X, y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y-1][X] == i) {
                                        corrPixels[j] = Point(X, y-1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y+1][X] == i) {
                                        corrPixels[j] = Point(X, y+1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // yが増える方向
                        } else {
                            for (int y = bp.y-2; y > ty; y--) {
                                
                                X = (int)round(a * y + b);
                                if (X < lx || X > rx) break;
                                
                                Scalar color = getColor(X, y);
                                tmp_colors.push_back(color);
                                
                                if (y > ty && y < by) {
                                    if (table[y][X] == i) {
                                        corrPixels[j] = Point(X, y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y-1][X] == i) {
                                        corrPixels[j] = Point(X, y-1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y+1][X] == i) {
                                        corrPixels[j] = Point(X, y+1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // yが減る方向
                        }
                    }
                } else {        // a<0
                    if (isPositive) {   // 勾配方向に探索
                        if (thetas[j] >= 0) {
                            for (int y = bp.y-2; y > ty; y--) {
                                
                                X = (int)round(a * y + b);
                                if (X < lx || X > rx) break;
                                
                                Scalar color = getColor(X, y);
                                tmp_colors.push_back(color);
                                
                                if (y > ty && y < by) {
                                    if (table[y][X] == i) {
                                        corrPixels[j] = Point(X, y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y-1][X] == i) {
                                        corrPixels[j] = Point(X, y-1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y+1][X] == i) {
                                        corrPixels[j] = Point(X, y+1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // yが減る方向
                        } else {
                            for (int y = bp.y+2; y < by; y++) {
                                
                                X = (int)round(a * y + b);
                                if (X < lx || X > rx) break;
                                
                                Scalar color = getColor(X, y);
                                tmp_colors.push_back(color);
                                
                                if (y > ty && y < by) {
                                    if (table[y][X] == i) {
                                        corrPixels[j] = Point(X, y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y-1][X] == i) {
                                        corrPixels[j] = Point(X, y-1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y+1][X] == i) {
                                        corrPixels[j] = Point(X, y+1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // yが増える方向
                        }
                    } else {            // 勾配方向と逆に探索
                        if (thetas[j] >= 0) {
                            for (int y = bp.y+2; y < by; y++) {
                                
                                X = (int)round(a * y + b);
                                if (X < lx || X > rx) break;
                                
                                Scalar color = getColor(X, y);
                                tmp_colors.push_back(color);
                                
                                if (y > ty && y < by) {
                                    if (table[y][X] == i) {
                                        corrPixels[j] = Point(X, y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y-1][X] == i) {
                                        corrPixels[j] = Point(X, y-1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y+1][X] == i) {
                                        corrPixels[j] = Point(X, y+1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // yが増える方向
                        } else {
                            for (int y = bp.y-2; y > ty; y--) {
                                
                                X = (int)round(a * y + b);
                                if (X < lx || X > rx) break;
                                
                                Scalar color = getColor(X, y);
                                tmp_colors.push_back(color);
                                
                                if (y > ty && y < by) {
                                    if (table[y][X] == i) {
                                        corrPixels[j] = Point(X, y);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y-1][X] == i) {
                                        corrPixels[j] = Point(X, y-1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    } else if (table[y+1][X] == i) {
                                        corrPixels[j] = Point(X, y+1);
                                        colors.insert(colors.end(), tmp_colors.begin(), tmp_colors.end());
                                        break;
                                    }
                                }
                            }   // yが減る方向
                        }
                    }
                }
            }
            
            if (pixels[j] == corrPixels[j]) {
                corrPixels[j] = Point(-1, -1);
            }
            objects[i].corrPairPixels = corrPixels;
            tmp_colors.clear();
        }
        
        objects[i].corrPairPixels = corrPixels;
        objects[i].isPositive = isPositive;
        objects[i].computeColor(colors);
    }
    for (int i = 0; i < srcImage.rows; ++i) {
        delete [] table[i];
    }
    delete [] table;
}

int** Mycv::createImageTable(const vector<Object>& objects)
{
    int **table = new int*[srcImage.rows];
    for (int i = 0; i < srcImage.rows; ++i) {
        table[i] = new int[srcImage.cols];
        for (int j = 0; j < srcImage.cols; j++) {
            table[i][j] = -1;
        }
    }
    
    cv::Point p;
    for (int i = 0; i < objects.size(); i++) {
        for (int j = 0; j < objects[i].contourPixels.size(); j++) {
            p = objects[i].contourPixels[j];
            table[p.y][p.x] = i;
        }
    }
    
    return table;
}

void Mycv::gradientOfCorrPairs(vector<Object>& objects, const Mat_<double>& gradients)
{
    cv::Point p;
    double t = 0;
    for (int i = 0; i < objects.size(); i++)
    {
        vector<double> *thetas = new vector<double>();
        for (int j = 0; j < objects[i].corrPairPixels.size(); j++)
        {
            p = objects[i].corrPairPixels[j];
            if (p.x < 0 || p.y < 0) {
                thetas->push_back(10);
            } else {
                t = gradients.at<double>(p.y, p.x);
                thetas->push_back(t);
            }
        }
        objects[i].corrThetas = *thetas;
    }
}

// Compute Features
void Mycv::computeEchar(vector<Object>& objects)
{
    double tempADGD = 0, ADGD = 0, FCP = 0, Echar = 0;
    double alpha = 0.6;
    int FCPcount = 0, count = 0;
    
    for (int i = 0; i < objects.size(); i++)
    {
        ADGD = 0;
        FCPcount = 0;
        count = 0;
            
        for (int j = 0; j < objects[i].thetas.size(); j++)
        {
            if (objects[i].corrPairPixels[j].x < 0 ||
                objects[i].corrPairPixels[j].y < 0) continue;
            
            if (fabs(objects[i].thetas[j]) <= M_PI && fabs(objects[i].corrThetas[j]) <= M_PI)
            {
                tempADGD = fabs(objects[i].thetas[j] - objects[i].corrThetas[j]);
                if (tempADGD > M_PI) tempADGD = 2 * M_PI - tempADGD;
                ADGD += tempADGD;
                if (tempADGD >= M_PI * alpha) FCPcount++;
            }
            
            count++;
        }
        
        if (count > 0)
        {
            // ADGDとFCPの算出
            ADGD = ADGD / count;
            objects[i].Gangle = ADGD;
            FCP = (double)FCPcount / (double)count;
            objects[i].Fcorr = FCP;
            // Echarの算出
            Echar = (ADGD / M_PI + FCP) / 2;
            objects[i].Echar = Echar;
        }
    }
}

// Compute Stroke Width
void Mycv::computeStrokeWidth(vector<Object>& objects)
{
    const int sampling = 2;
    
    int max = 0;
    double tmp_w = 0;
    vector<int> hist;
    vector<cv::Point> basePixels, pairPixels;
    cv::Point diff;
    
    for (int i = 0; i < objects.size(); i++) {
        
        max = MAX(objects[i].width, objects[i].height);
        
        hist = *new vector<int>(max, 0);
        basePixels = objects[i].contourPixels;
        pairPixels = objects[i].corrPairPixels;
        
        for (int j = 0; j < basePixels.size(); j++) {
            if (pairPixels[j].x>=0 && pairPixels[j].y>=0) {
                diff = basePixels[j] - pairPixels[j];
                tmp_w = round(sqrt(diff.x*diff.x+diff.y*diff.y));
                hist[(int)floor(tmp_w / sampling)]++;
            }
        }
        
        int peak = 0, maxK = 0;
        for (int k = 0; k < max; k++) {
            if (hist[k] > peak) {
                peak = hist[k];
                maxK = k;
            }
        }
        
        objects[i].strokeWidth = maxK*sampling;
    }
}

// Set Features to the Object
void Mycv::setFeatures(vector<Object>& objects)
{
    for (int i = 0; i < objects.size(); i++) {
        
        vector<double> features;
        
        // Echar
        /* 0 */ features.push_back(objects[i].Gangle / M_PI);
        /* 1 */ features.push_back(objects[i].Fcorr);
        /* 2 */ features.push_back(objects[i].Echar);
        
        // Color
        /* 3 */ features.push_back((double)objects[i].color[0]/BRIGHTNESS);
        /* 4 */ features.push_back((double)objects[i].color[1]/BRIGHTNESS);
        /* 5 */ features.push_back((double)objects[i].color[2]/BRIGHTNESS);
        
        // Stroke width
        /* 6 */ features.push_back(objects[i].strokeWidth/objects[i].longLength);
        
        
        // TODO: Stroke width の分散
        
        // TODO: Contour roughness
        
        
        // Rect ratio
        /* 7 */ features.push_back(objects[i].rectRatio);
        
        // Aspect ratio
        /* 8 */ features.push_back(objects[i].aspectRatio);
        
        // Long length ratio
        /* 9 */ features.push_back(objects[i].longLengthRatio);
        
        // Area ratio
        /* 10 */ features.push_back(objects[i].areaRatio);
        
        
        // Set features
        objects[i].features = features;
    }
}


#pragma mark -
#pragma mark Grouping Objects Methods

void Mycv::groupingObjects(Text& text, Object& obj, vector<double>& distance, vector<Object>& objects)
{
    recursive_count++;
    //cout<< "Recursive Count:" << recursive_count << endl;
    
    vector<Object*> neighbors;
    findNeighbors(neighbors, obj, distance, objects);
    adding(text, neighbors);
    
    for (int i = 0; i < neighbors.size(); i++) {
        groupingObjects(text, *neighbors[i], distance, objects);
    }
}

double Mycv::distanceWithNearestNeighbor(Object& obj, vector<Object>& objects)
{
    double min = INFINITY;
    for (int i = 0; i < objects.size(); i++)
    {
        if (obj.ID == objects[i].ID) continue;
        double d = distanceOfObjects(obj, objects[i]);
        if (min > d) min = d;
    }
    return min;
}

void Mycv::findNeighbors(vector<Object*>& neighbors, Object& obj, vector<double>& distance, vector<Object>& objects)
{
    double temp_dist = 0, average = 0;
    
    for (int i = 0; i < objects.size(); i++)
    {
        average = 0;
        cout << objects[i].grouped << endl;
        if (obj != objects[i] && !(objects[i].grouped))
        {
            average = accumulate(distance.begin(), distance.end(), 0);
            average /= (double)distance.size();
            temp_dist = distanceOfObjects(obj, objects[i]);
            
            if (temp_dist < average) {
                if (obj.neighborDistance > temp_dist) {
                    obj.neighborDistance = temp_dist;
                    if (obj.neighbors.size() > 0) obj.neighbors.pop_back();
                    obj.neighbors.push_back(objects[i]);
                }
                neighbors.push_back(&objects[i]);
                distance.push_back(temp_dist);
            }
        }
    }
}

double Mycv::distanceOfObjects(const Object& obj1, const Object& obj2)
{
    cv::Point diff1 = obj2.centroid - obj1.centroid;
    cv::Point diff2 = obj1.centroid - obj2.centroid;
    
    double a = (double)diff1.y / diff1.x;
    
    double t1 = atan2(-diff1.y, diff1.x);
    double t2 = atan2(-diff2.y, diff2.x);
    
    int pattern1 = patternOfRadian(t1);
    int pattern2 = patternOfRadian(t2);
    
    cv::Point rep1;
    findRepresentativePoint(rep1, obj1, a, pattern1);
    cv::Point rep2;
    findRepresentativePoint(rep2, obj2, a, pattern2);
    
    double d = distanceOfPoints(rep1, rep2);
    cout << "Obj1:" << obj1.centroid << ", Rep1:" << rep1 << ", Obj2:" << obj2.centroid << ", Rep2:"
         << rep2 << ", D:" << d << endl;
    
    return d;
}

int Mycv::patternOfRadian(const double radian)
{
    int n = 0;
    if (radian <= M_PI && radian > M_PI_2) n = 2;
    else if (radian <= M_PI_2 && radian > 0) n = 1;
    else if (radian <= 0 && radian > -M_PI_2) n = 4;
    else  n = 3;
    
    return n;
}

double Mycv::distanceOfPoints(const cv::Point& p1, const cv::Point& p2)
{
    cv::Point diff = p1 - p2;
    return sqrt(diff.x*diff.x+diff.y*diff.y);
}

void Mycv::pointApartFromCentroid(cv::Point& rep, const cv::Point& centroid, const cv::Point& p1, const cv::Point& p2, const double a)
{
    double b = centroid.y - a * centroid.x;
    cv::Point tmp_p1, tmp_p2;
    double tmp_a = 1 / a;
    double tmp_b1 = 0, tmp_b2 = 0;
    
    tmp_b1 = p1.y - tmp_a * p1.x;
    tmp_p1.x = - (b - tmp_b1) / (a - tmp_a);
    tmp_p1.y = a * tmp_p1.x + b;
    
    tmp_b2 = p2.y - tmp_a * p2.x;
    tmp_p2.x = - (b - tmp_b2) / (a - tmp_a);
    tmp_p2.y = a * tmp_p2.x + b;
    
    cout << "centroid:" << centroid << ", rep1:" << tmp_p1 << ", rep2:" << tmp_p2 << endl;
    if (distanceOfPoints(tmp_p1, centroid) > distanceOfPoints(tmp_p2, centroid)) rep = tmp_p1;
    else rep = tmp_p2;
}

void Mycv::findRepresentativePoint(cv::Point& rep, const Object& obj, const double a, const int pattern)
{
    switch (pattern) {
        case 1: // tp か rp を採用
            pointApartFromCentroid(rep, obj.centroid, obj.tp, obj.rp, a);
            break;
        case 2: // tp - lp を採用
            pointApartFromCentroid(rep, obj.centroid, obj.tp, obj.lp, a);
            break;
        case 3: // lp - bp を採用
            pointApartFromCentroid(rep, obj.centroid, obj.bp, obj.lp, a);
            break;
        case 4: // rp - bp を採用
            pointApartFromCentroid(rep, obj.centroid, obj.bp, obj.rp, a);
            break;
    }
}

void Mycv::adding(Text& text, vector<Object*>& neighbors)
{
//    vector<int> rmIndex;
    for (int i = 0; i < neighbors.size(); i++)
    {
        Object* n = neighbors[i];
        if (n->grouped) continue;
        
        text.add(*n);
        n->grouped = true;
        
//        if (isSimilar(text.color, n->color))
//        {
//            text.add(*n);
//            n->grouped = true;
//        }
//        else
//        {
//            rmIndex.push_back(i);
//        }
    }
//    for (int i = (int)rmIndex.size()-1; i > -1; i--) {
//        neighbors.erase(neighbors.begin()+rmIndex[i]);
//    }
}

bool Mycv::isSimilar(Scalar c1, Scalar c2)
{
    Scalar diff = c1 - c2;
    double dist = sqrt(diff[0]*diff[0]+diff[1]*diff[1]+diff[2]*diff[2]);
    if (dist <= 50) return true;
    
    return false;
}


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



Mycv::~Mycv()
{
    
}

