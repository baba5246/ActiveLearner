
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

inline bool isSimilarLab(Object*& obj1, Object*& obj2)
{
    Scalar a = obj1->labcolor;
    Scalar b = obj2->labcolor;
    double dl = a.val[0]-b.val[0];
    double da = a.val[1]-b.val[1];
    double db = a.val[2]-b.val[2];
    return sqrt(dl*dl+da*da+db*db) < LAB_COLOR_SIMILARITY;
}

inline bool isSimilarLab(Text*& t1, Text*& t2)
{
    Scalar a = t1->labcolor;
    Scalar b = t2->labcolor;
    double dl = a.val[0]-b.val[0];
    double da = a.val[1]-b.val[1];
    double db = a.val[2]-b.val[2];
    return sqrt(dl*dl+da*da+db*db) < LAB_COLOR_SIMILARITY;
}

#pragma mark -
#pragma mark Interface Methods

void TextDetector::detect(vector<Object*>& objects, vector<Text*>& texts)
{
    vector<Text*> candidate_texts, filtered_texts;
    
    // Group 抽出
    detectTexts(candidate_texts, objects);
    
    // Group特徴量計算
    setFeatures(candidate_texts, objects);
    
    // Linkの数でフィルタリング
    textFiltering(filtered_texts, candidate_texts);
    
    // Draw
    Draw::draw(Draw::drawTexts(srcImage, filtered_texts));
    
    // Groupのマージ
    mergeFilteredTexts(texts, filtered_texts);
    
    // Group特徴量計算
    setFeatures(texts, objects);
    
}


#pragma mark -
#pragma mark Grouping Objects Methods

void TextDetector::detectTexts(vector<Text*>& texts, vector<Object*>& objects)
{
    // Sort
    sort(objects.begin(), objects.end(), Object::LTtoRB);
    
    // Grouping アルゴリズム
    for (int i = 0; i < objects.size(); i++)
    {
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
        
        // 距離が閾値以下で
        if (temp_dist < threshold) {
            // Lab色空間で類似しているなら
            if (isSimilarLab(init, objects[i])) {
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

void TextDetector::textFiltering(vector<Text*>& dst_texts, vector<Text*>& src_texts)
{
    int count = 0;
    for (int i = 0; i < src_texts.size(); i++)
    {
        // Link数でフィルタリング
        bool link_out = false;
        
        vector<int> originIndexes(src_texts[i]->originIndexes);
        sort(originIndexes.begin(), originIndexes.end());
        for (int j = 0; j < originIndexes.size()-1; j++)
        {
            if (originIndexes[j] == originIndexes[j+1]) {
                count++;
            } else {
                count = 0;
            }
            
            if (count > SAME_LINK_COUNT) {
                link_out = true;
                break;
            }
        }
        
        if (link_out == false) {
            dst_texts.push_back(src_texts[i]);
//            Draw::draw(Draw::drawText(srcImage, src_texts[i]));
        }
    }
}

// Merge texts
void TextDetector::mergeFilteredTexts(vector<Text*>& dst_texts, vector<Text*>& src_texts)
{
    sort(src_texts.begin(), src_texts.end(), Text::isLeftLarge);
    
    vector<int> alreadies;
    for (int i = 0; i < src_texts.size(); i++) {
        
        if (find(alreadies.begin(), alreadies.end(), i) != alreadies.end())
            continue;
        
        Text *text(src_texts[i]);
        Rect large = src_texts[i]->rect;
        
        for (int j = i+1; j < src_texts.size(); j++) {
            
            if (find(alreadies.begin(), alreadies.end(), j) != alreadies.end())
                continue;
            
            Rect small = src_texts[j]->rect;
            Rect intersect = small & large;
            double swratio = 0, lwratio = 0, shratio = 0, lhratio = 0, saratio = 0, laratio = 0;
            
            if (intersect.width>0 && intersect.height) {
                swratio = (double)intersect.width / small.width;
                lwratio = (double)intersect.width / large.width;
                shratio = (double)intersect.height / small.height;
                lhratio = (double)intersect.height / large.height;
                saratio = (double)intersect.area() / small.area();
                laratio = (double)intersect.area() / large.area();
                
                // 一部でも重なっているものが対象
                if (intersect.area()>0)
                {
                    // 横長いもの同士が対象
                    if (small.width>small.height && large.width>large.height)
                    {
                        // 重なってる割合が両方大きい
                        if (shratio > RECT_HABA_MARGE_THRESHOLD &&
                            lhratio > RECT_HABA_MARGE_THRESHOLD)
                        {
                            // 平均色が類似している
                            if (isSimilarLab(src_texts[i], src_texts[j])) {
                                text->add(src_texts[j]);
                                alreadies.push_back(j);
                                continue;
                            }
                        }
                    }
                    // 縦長いもの同士が対象
                    else if (small.width<small.height && large.width<large.height)
                    {
                        // 重なってる割合が両方大きい
                        if (swratio > RECT_HABA_MARGE_THRESHOLD &&
                            lwratio > RECT_HABA_MARGE_THRESHOLD)
                        {
                            // 平均色が類似している
                            if (isSimilarLab(src_texts[i], src_texts[j])) {
                                text->add(src_texts[j]);
                                alreadies.push_back(j);
                                continue;
                            }
                        }

                    }
                }
                
            }
        }
        
        text->reLinkOriginIndexes();
        dst_texts.push_back(text);
//        Draw::draw(Draw::drawText(srcImage, text));
    }
    
}

// Merge texts
void TextDetector::mergeContainedTexts(vector<Text*>& dst_texts, vector<Text*>& src_texts)
{
    sort(src_texts.begin(), src_texts.end(), Text::isLeftLarge);
    
    vector<int> alreadies;
    for (int i = 0; i < src_texts.size(); i++) {
        
        if (find(alreadies.begin(), alreadies.end(), i) != alreadies.end())
            continue;
        
        Text *text(src_texts[i]);
        Rect large = src_texts[i]->rect;
        
        for (int j = i+1; j < src_texts.size(); j++) {
            
            if (find(alreadies.begin(), alreadies.end(), j) != alreadies.end())
                continue;
            
            Rect small = src_texts[j]->rect;
            Rect intersect = small & large;
            double saratio = 0, laratio = 0;
            
            if (intersect.width>0 && intersect.height) {
                saratio = (double)intersect.area() / small.area();
                laratio = (double)intersect.area() / large.area();
                
                // 一部でも重なっているものが対象
                if (intersect.area()>0)
                {
                    // 横長いもの同士が対象
                    if (saratio > 0.99)
                    {
                        text->add(src_texts[j]);
                        alreadies.push_back(j);
                        continue;
                    }
                }
                
            }
        }
        
        text->reLinkOriginIndexes();
        dst_texts.push_back(text);
//        Draw::draw(Draw::drawText(srcImage, text));
    }
    
}

// Set Features to the Object
void TextDetector::setFeatures(vector<Text*>& texts, vector<Object*>& objects)
{
    long size = texts.size();
    double maxSW = 0, maxAngle = 0, maxDist = 0;
    for (int i = 0; i < size; i++) {
        
        // Compute Features
        texts[i]->computeProperties();
        
        // Compute Max Values
        if (maxSW < texts[i]->varSW) maxSW = texts[i]->varSW;
        if (maxAngle < texts[i]->varAngle) maxAngle = texts[i]->varAngle;
        if (maxDist < texts[i]->varDist) maxDist = texts[i]->varDist;
        
        // Count False Contain Objects
        texts[i]->countFalseObjects(objects);
    }
    
    for (int i = 0; i < size; i++) {
        
        vector<double> features;
        
        // Average Features
        /* 0 */ features.push_back(texts[i]->aveGangle / M_PI);
        /* 1 */ features.push_back(texts[i]->aveFcorr);
        /* 2 */ features.push_back(texts[i]->aveEchar);
        /* 3 */ features.push_back(texts[i]->aveColorSim);
        /* 4 */ features.push_back(texts[i]->aveLabSim);
        /* 5 */ features.push_back(texts[i]->aveSW);
        /* 6 */ features.push_back(texts[i]->aveDist);
        /* 7 */ features.push_back(texts[i]->aveAngle);
        /* 8 */ features.push_back(texts[i]->aveAspect);
        /* 9 */ features.push_back(texts[i]->aveCircleR);
        
        // Variant Features
        /* 10 */ features.push_back(texts[i]->varAngle);
        /* 11 */ features.push_back(texts[i]->varDist);
        /* 12 */ features.push_back(texts[i]->varHeight);
        /* 13 */ features.push_back(texts[i]->varSW);
        /* 14 */ features.push_back(texts[i]->varWidth);

        // Ratio Features
        /* 15 */ features.push_back(texts[i]->rectRatio);
        /* 16 */ features.push_back(texts[i]->aspectRatio);
        /* 17 */ features.push_back(texts[i]->longLengthRatio);
        /* 18 */ features.push_back(texts[i]->objAreaRatio);
        /* 19 */ features.push_back(texts[i]->objSizeRatio);
        /* 20 */ features.push_back(texts[i]->trueObjectRatio);
        
        // Set features
        texts[i]->features = features;
    }
}

