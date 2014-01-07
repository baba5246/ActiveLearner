
#include "ObjectDetector.h"

vector<string> ObjectDetector::split(const string& str, char delim)
{
    istringstream iss(str); string tmp; vector<string> res;
    while(getline(iss, tmp, delim)) res.push_back(tmp);
    return res;
}


#pragma mark -
#pragma mark Constructors

ObjectDetector::ObjectDetector()
{
    
}

ObjectDetector::ObjectDetector(const string& filepath)
{
    srcImage = imread(filepath, CV_LOAD_IMAGE_COLOR);
    srcIplImage = srcImage;
    
    vector<string> comps = split(filepath, '/');
    filename = comps[comps.size()-1];
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
    mycv.grayscale(srcImage, rgray, MYCV_GRAY_R);
    mycv.grayscale(srcImage, ggray, MYCV_GRAY_G);
    mycv.grayscale(srcImage, bgray, MYCV_GRAY_B);
    mycv.canny(rgray, redge);
    mycv.canny(ggray, gedge);
    mycv.canny(bgray, bedge);
    mycv.mergeEdges(redge, gedge, bedge, edge);
    
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
    cout << "merge apart contours:" << objects.size() << endl;
    if (objects.size() == 0) return;
    mergeIncludedObjects(objects);
    
    // SWT と Color 類似度を使って補正
    Mat_<double> gradient = Mat_<double>(srcImage.rows, srcImage.cols);
    mycv.sobelFiltering(ggray, gradient);
    
    // Extract connected regions as components
    Mat swtm, swtp;
    vector<vector<cv::Point> > compm, compp;
    vector<SWTObject> swtobjects;
    mycv.SWTMinus(gedge, gradient, swtm);
    mycv.SWTComponents(swtm, compm);
    mycv.SWTPlus(gedge, gradient, swtp);
    mycv.SWTComponents(swtp, compp);
    createSWTObjects(swtobjects, swtp, compm);
    createSWTObjects(swtobjects, swtp, compp);
    
    Draw::drawSWTObjects(swtm, swtobjects);
    
    // Object の勾配方向計算
    gradientOfObjects(objects, gradient);
    
//    Draw::drawGradients(objects, gradient); // 勾配方向描画
    
    // Corresponding Pair の探索
    findCorrPairs(objects, gradient);
    gradientOfCorrPairs(objects, gradient);
    
    // 特徴量計算
    computeEchar(objects);          // Character Energy
    computeStrokeWidth(objects);    // Stroke Width
    setFeatures(objects);           // 特徴量をセット
    
//    Draw::drawEchars(srcImage, objects); // Echar描画
    
    vector<Text*> texts;
    TextDetector detector(srcImage);
    detector.detect(objects, texts);
    
//    Draw::drawTexts(srcImage, texts);
    
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
        Object *obj = new Object(Id.str(), filename, contours[i], cv::Size(srcImage.cols, srcImage.rows));
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
    
    double wratio = 0, hratio = 0, lratio = 0;
    vector<int> removeIndexes;
    
    cout << "merge included objects:" << objects.size() << endl;
    for (int i = 0; i < objects.size()-1; i++)
    {
        largeRect = objects[i]->rect;
        for (int j = i+1; j < objects.size(); j++)
        {
            smallRect = objects[j]->rect;
            interRect = *intersect(largeRect, smallRect);
            if (interRect.area() == 0) continue;
            
            wratio = (double)max(smallRect.width, largeRect.width) / min(smallRect.width, largeRect.width);
            hratio = (double)max(smallRect.height, largeRect.height) / min(smallRect.height, largeRect.height);
            lratio = objects[i]->longLength / objects[j]->longLength;
            
            if (lratio < 4)//wratio < 4 || hratio < 4)
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

// Find Corresponding Pairs
void ObjectDetector::findCorrPairs(vector<Object*>& objects, const Mat& gradients)
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
        pixels = objects[i]->contourPixels;
        thetas = objects[i]->thetas;
        corrPixels = *new vector<Point>(pixels.begin(), pixels.end());
        colors = *new vector<Scalar>();
        
        ty = objects[i]->rect.tl().y;
        lx = objects[i]->rect.tl().x;
        rx = objects[i]->rect.br().x;
        by = objects[i]->rect.br().y;
        
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
            objects[i]->corrPairPixels = corrPixels;
            tmp_colors.clear();
        }
        
        objects[i]->corrPairPixels = corrPixels;
        objects[i]->isPositive = isPositive;
        objects[i]->computeColor(colors);
    }
    for (int i = 0; i < srcImage.rows; ++i) {
        delete [] table[i];
    }
    delete [] table;
}

int** ObjectDetector::createImageTable(const vector<Object*>& objects)
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
        for (int j = 0; j < objects[i]->contourPixels.size(); j++) {
            p = objects[i]->contourPixels[j];
            table[p.y][p.x] = i;
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
        vector<double> *thetas = new vector<double>();
        for (int j = 0; j < objects[i]->corrPairPixels.size(); j++)
        {
            p = objects[i]->corrPairPixels[j];
            if (p.x < 0 || p.y < 0) {
                thetas->push_back(10);
            } else {
                t = gradients.at<double>(p.y, p.x);
                thetas->push_back(t);
            }
        }
        objects[i]->corrThetas = *thetas;
    }
}

// Compute Features
void ObjectDetector::computeEchar(vector<Object*>& objects)
{
    double tempADGD = 0, ADGD = 0, FCP = 0, Echar = 0;
    double alpha = 0.6;
    int FCPcount = 0, count = 0;
    
    for (int i = 0; i < objects.size(); i++)
    {
        ADGD = 0;
        FCPcount = 0;
        count = 0;
        
        for (int j = 0; j < objects[i]->thetas.size(); j++)
        {
            if (objects[i]->corrPairPixels[j].x < 0 ||
                objects[i]->corrPairPixels[j].y < 0) continue;
            
            if (fabs(objects[i]->thetas[j]) <= M_PI && fabs(objects[i]->corrThetas[j]) <= M_PI)
            {
                tempADGD = fabs(objects[i]->thetas[j] - objects[i]->corrThetas[j]);
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
            objects[i]->Gangle = ADGD;
            FCP = (double)FCPcount / (double)count;
            objects[i]->Fcorr = FCP;
            // Echarの算出
            Echar = (ADGD / M_PI + FCP) / 2;
            objects[i]->Echar = Echar;
        }
    }
}

// Compute Stroke Width
void ObjectDetector::computeStrokeWidth(vector<Object*>& objects)
{
    const int sampling = 2;
    
    int max = 0;
    double tmp_w = 0;
    vector<int> hist;
    vector<cv::Point> basePixels, pairPixels;
    cv::Point diff;
    
    for (int i = 0; i < objects.size(); i++) {
        
        max = MAX(objects[i]->width, objects[i]->height);
        
        hist = *new vector<int>(max, 0);
        basePixels = objects[i]->contourPixels;
        pairPixels = objects[i]->corrPairPixels;
        
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
        
        objects[i]->strokeWidth = maxK*sampling;
    }
}

// Set Features to the Object
void ObjectDetector::setFeatures(vector<Object*>& objects)
{
    for (int i = 0; i < objects.size(); i++) {
        
        vector<double> features;
        
        // Echar
        /* 0 */ features.push_back(objects[i]->Gangle / M_PI);
        /* 1 */ features.push_back(objects[i]->Fcorr);
        /* 2 */ features.push_back(objects[i]->Echar);
        
        // Color
        /* 3 */ features.push_back((double)objects[i]->color[0]/BRIGHTNESS);
        /* 4 */ features.push_back((double)objects[i]->color[1]/BRIGHTNESS);
        /* 5 */ features.push_back((double)objects[i]->color[2]/BRIGHTNESS);
        
        // Stroke width
        /* 6 */ features.push_back(objects[i]->strokeWidth/objects[i]->longLength);
        
        
        // TODO: Stroke width の分散
        
        // TODO: Contour roughness
        
        
        // Rect ratio
        /* 7 */ features.push_back(objects[i]->rectRatio);
        
        // Aspect ratio
        /* 8 */ features.push_back(objects[i]->aspectRatio);
        
        // Long length ratio
        /* 9 */ features.push_back(objects[i]->longLengthRatio);
        
        // Area ratio
        /* 10 */ features.push_back(objects[i]->areaRatio);
        
        
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

void ObjectDetector::SWTComponent(const Mat& src, vector<Object>& component)
{
    
}



