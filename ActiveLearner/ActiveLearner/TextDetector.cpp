
#include "TextDetector.h"

#pragma mark -
#pragma mark Constructors

TextDetector::TextDetector()
{
    
}

TextDetector::TextDetector(const Mat& src)
{
    srcImage = src;
}

TextDetector::~TextDetector()
{
    
}

#pragma mark -
#pragma mark Inline Methods


inline double computeGradient(Object*& obj1, Object*& obj2)
{
    cv::Point diff = obj1->centroid - obj2->centroid;
    return atan2(-diff.y, diff.x);
}

inline double pointSize(cv::Point p)
{
    return sqrt(p.x*p.x+p.y*p.y);
}

inline double distanceObjects(Object*& obj1, Object*& obj2)
{
    Point diff = obj1->centroid - obj2->centroid;
    return pointSize(diff);
}



#pragma mark -
#pragma mark Interface Methods

void TextDetector::detect(vector<Object*>& objects, vector<Text*>& texts)
{
    vector<Text*> temp_texts;
    
    // Group 抽出
    detectTexts(temp_texts, objects);
    
    // Group特徴量計算
    setFeatures(temp_texts);
    
    // Groupのマージ
    mergeTempTexts(texts, temp_texts);
    
    // Group特徴量計算
    setFeatures(texts);
    
    
    Draw::draw(Draw::drawTexts(srcImage, texts));
}


#pragma mark -
#pragma mark Grouping Objects Methods

void TextDetector::detectTexts(vector<Text*>& texts, vector<Object*>& objects)
{
    // Sort
    sort(objects.begin(), objects.end(), Object::leftToRight);
    
    // Grouping アルゴリズム
    for (int i = 0; i < objects.size(); i++)
    {
        // Echarが低すぎるやつは排除
        if (objects[i]->Echar < 0.50f) continue;
        
        Object *init = objects[i];
        
        // Find neighbors
        vector<Object*> neighbors = findInitNeighbors(init, objects);
        
        // For all neighbors
        for (int j = 0; j < neighbors.size(); j++) {
            
            // Create groups
            Text *text = new Text(init->filepath, init);
            text->focusedIndex = 0;
            
            // Find surroundings
            text->add(neighbors[j], distanceObjects(init, neighbors[j]));
            double gradient = computeGradient(init, neighbors[j]);
            text->gradients.push_back(gradient);
            text->focusedIndex = 1;
            
            // Grouping
            groupingObjects(text, objects);
            
            // Add as a candidate
            texts.push_back(text);
            
            //            Draw::draw(Draw::drawText(srcImage, text));
        }
    }
}

vector<Object*> TextDetector::findInitNeighbors(Object*& init, vector<Object*>& objects)
{
    vector<Object*> neighbors;
    double temp_dist = 0, gradient = 0;
    cv::Point vec1, vec2;
    int existIndex = -1;
    
    // 距離閾値
    double threshold = init->longLength*2;
    
    for (int i = 0; i < objects.size(); i++)
    {
        // ID等しいなら continue;
        if (!init->ID.compare(objects[i]->ID)) continue;
        
        // 距離計算
        temp_dist = distanceObjects(init, objects[i]);
        
        // 閾値以下なら
        if (temp_dist < threshold) {
            
            // 既に見つけたものと同じ方向かどうか判定
            existIndex = -1;
            vec1 = objects[i]->centroid - init->centroid;
            for (int n = 0; n < neighbors.size(); n++) {
                vec2 = neighbors[n]->centroid - init->centroid;
                gradient = acos(vec1.ddot(vec2) / (pointSize(vec1) * pointSize(vec2)));
                if (fabs(gradient) < M_PI_1_8) {
                    existIndex = n;
                    break;
                }
            }
            
            // 同じ方向のものがなければneighborsとして入れる
            if (existIndex<0)
            {
                neighbors.push_back(objects[i]);
            }
            else // そうでなければ一番近いものだけ入れる
            {
                if (temp_dist < distanceObjects(init, neighbors[existIndex])) {
                    neighbors.erase(neighbors.begin()+existIndex);
                    neighbors.push_back(objects[i]);
                }
            }
        }
    
    }
    
    return neighbors;
}

void TextDetector::groupingObjects(Text*& text, vector<Object*>& objects)
{
    // 注目indexがオブジェクト数より大きくなるなら終了
    if (text->focusedIndex >= text->objects.size()) return;
    
    // 周辺オブジェクトのうち、条件を満たすものはグループに追加する
    addNeighbors(text, objects);
    
    // インクリメントして再帰
    text->focusedIndex++;
    groupingObjects(text, objects);
}

void TextDetector::addNeighbors(Text*& text, vector<Object*>& objects)
{
    double distance = 0, threshold = 0, gradient = 0, swratio = 0;
    cv::Point diff, newVector, oldVector;
    int count = 0;
    
    // 注目オブジェクト取得
    Object *focus = text->objects[text->focusedIndex];
    
    for (int i = 0; i < objects.size(); i++) {
        
        if (!focus->ID.compare(objects[i]->ID)) continue;
        if (text->contains(objects[i])) continue;
        
        // 距離計算
        distance = distanceObjects(focus, objects[i]);
        
        // Text内のObject数に応じて閾値計算
        double ratio = (1 + pow(1.2, -((double)text->objects.size()-2)));
        threshold = text->aveDist * ratio;
        
        // 閾値内判定
        if (distance < threshold) {
            
            // ベクトル勾配計算
            Object *origin = text->objects[text->originIndexes[text->focusedIndex]];
            oldVector = origin->centroid - focus->centroid;
            newVector = objects[i]->centroid - focus->centroid;
            gradient = acos(newVector.ddot(oldVector) / (pointSize(newVector)*pointSize(oldVector)));
            // Stroke幅比計算
            swratio = MAX(focus->strokeWidth, text->aveSW) / MIN(focus->strokeWidth, text->aveSW);
            
            // 条件判定
            if (gradient > HIGH_GRADIENT_THRESHOLD && swratio < STROKE_WIDTH_RATIO)
            {
                // 追加
                text->add(objects[i], distance);
                text->gradients.push_back(gradient);
//                Draw::drawText(srcImage, text);
            }
            
            count++;
        }
    }
}


// Set Features to the Object
void TextDetector::setFeatures(vector<Text*>& texts)
{
    long size = texts.size();
    double maxSW = 0, maxR = 0, maxG = 0, maxB = 0, maxAngle = 0, maxDist = 0;
    for (int i = 0; i < size; i++) {
        
        // Compute Features
        texts[i]->computeProperties();
        
        // Compute Max Values
        if (maxSW < texts[i]->varSW) maxSW = texts[i]->varSW;
        if (maxR < texts[i]->varColorR) maxR = texts[i]->varColorR;
        if (maxG < texts[i]->varColorG) maxG = texts[i]->varColorG;
        if (maxB < texts[i]->varColorB) maxB = texts[i]->varColorB;
        if (maxAngle < texts[i]->varAngle) maxAngle = texts[i]->varAngle;
        if (maxDist < texts[i]->varDist) maxDist = texts[i]->varDist;
    }
    
    for (int i = 0; i < size; i++) {
        
        vector<double> features;
        
        // Average Features
        /* 0 */ features.push_back(texts[i]->aveGangle / M_PI);
        /* 1 */ features.push_back(texts[i]->aveFcorr);
        /* 2 */ features.push_back(texts[i]->aveEchar);
        
        // Variant Features
        /* 3 */ features.push_back(1 - (texts[i]->varSW / maxSW));
        /* 4 */ features.push_back(1 - (texts[i]->varColorR / maxR));
        /* 5 */ features.push_back(1 - (texts[i]->varColorG / maxG));
        /* 6 */ features.push_back(1 - (texts[i]->varColorB / maxB));
        /* 7 */ features.push_back(1 - (texts[i]->varAngle / maxAngle));
        /* 8 */ features.push_back(1 - (texts[i]->varDist / maxDist));
        /* 9 */ features.push_back(texts[i]->varLength);
        
        // Rect ratio
        /* 10 */ features.push_back(texts[i]->rectRatio);
        
        // Aspect ratio
        /* 11 */ features.push_back(texts[i]->aspectRatio);
        
        // Long length ratio
        /* 12 */ features.push_back(texts[i]->longLengthRatio);
        
        // Object Area ratio
        /* 13 */ features.push_back(texts[i]->objAreaRatio);
        
        // Object Area ratio
        /* 14 */ features.push_back(texts[i]->objSizeRatio);
        
        
        // Set features
        texts[i]->features = features;
    }
}

// Merge texts
void TextDetector::mergeTempTexts(vector<Text*>& texts, vector<Text*>& temp_texts)
{
    vector<int> alreadies;
    for (int i = 0; i < temp_texts.size(); i++) {
        
        if (find(alreadies.begin(), alreadies.end(), i) != alreadies.end())
            continue;
        
        Text *text(temp_texts[i]);
        Rect rect = temp_texts[i]->rect;
        
        for (int j = i+1; j < temp_texts.size(); j++) {
            
            if (find(alreadies.begin(), alreadies.end(), j) != alreadies.end())
                continue;
            
            Rect temp_rect = temp_texts[j]->rect;
            Rect intersect = rect & temp_rect;
            double wratio_orig = 0, wratio_temp = 0, hratio_orig = 0, hratio_temp = 0;
            double wratio = 0, hratio = 0;
            
            if (intersect.width>0 && intersect.height) {
                wratio_orig = intersect.width / rect.width;
                wratio_temp = intersect.width / temp_rect.width;
                hratio_orig = intersect.height / rect.height;
                hratio_temp = intersect.height / temp_rect.height;
                
                // Almost Containのものをマージする
                if (rect.area() > temp_rect.area()) {
                    wratio = wratio_orig;
                    hratio = hratio_orig;
                } else {
                    wratio = wratio_temp;
                    hratio = hratio_temp;
                }
                if (wratio > ALMOST_CONTAIN_THRESHOLD && hratio > ALMOST_CONTAIN_THRESHOLD) {
                    text->add(temp_texts[j]);
                    alreadies.push_back(j);
                    continue;
                }
                
                if (rect.width > rect.height) {
                    if (temp_rect.width > temp_rect.height) {
                        if (wratio_orig > RECT_MARGE_THRESHOLD1 || wratio_temp > RECT_MARGE_THRESHOLD1) {
                            if (hratio_orig > RECT_MARGE_THRESHOLD2 && hratio_temp > RECT_MARGE_THRESHOLD2) {
                                text->add(temp_texts[j]);
                                alreadies.push_back(j);
                                continue;
                            }
                        }
                    }
                } else {
                    if (temp_rect.height > temp_rect.width) {
                        if (hratio_orig > RECT_MARGE_THRESHOLD1 || hratio_temp > RECT_MARGE_THRESHOLD1) {
                            if (wratio_orig > RECT_MARGE_THRESHOLD2 && wratio_temp > RECT_MARGE_THRESHOLD2) {
                                text->add(temp_texts[j]);
                                alreadies.push_back(j);
                                continue;
                            }
                        }
                    }
                }
            }
        }
        
        texts.push_back(text);
    }

}
