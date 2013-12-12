
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
#pragma mark Interface Methods

void TextDetector::detect(vector<Object*>& objects, vector<Text*>& texts)
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
            Text *text = new Text(init->filename, init);
            text->focusedIndex = 0;
            
            // Find surroundings
            text->add(neighbors[j], Distancer::distanceOfCentroids(init->centroid, neighbors[j]->centroid));
            double gradient = computeGradient(*init, *neighbors[j]);
            text->gradients.push_back(gradient);
            text->focusedIndex = 1;
            
            // Grouping
            groupingObjects(text, objects);
            
            // Add as a candidate
            texts.push_back(text);
            
            //Draw::drawTexts(srcImage, texts);
        }
    }
}


#pragma mark -
#pragma mark Grouping Objects Methods


vector<Object*> TextDetector::findInitNeighbors(Object* init, vector<Object*> objects)
{
    vector<Object*> neighbors;
    double temp_dist = 0, gradient = 0;
    double threshold = init->longLength;
    cv::Point vec1, vec2;
    bool exist = 0;
    
    for (int i = 0; i < objects.size(); i++)
    {
        // ID等しくないなら
        if (init->ID != objects[i]->ID)
        {
            temp_dist = Distancer::distanceOfObjects(*init, *objects[i]);
            //distanceOfCentroids(init->centroid, objects[i]->centroid);
            
            // 閾値以下なら
            if (temp_dist < threshold) {
                
                // 既に見つけたものと同じ方向かどうか判定
                exist = false;
                vec1 = objects[i]->centroid - init->centroid;
                for (int n = 0; n < neighbors.size(); n++) {
                    vec2 = neighbors[n]->centroid - init->centroid;
                    gradient = acos(vec1.ddot(vec2) / (pointSize(vec1) * pointSize(vec2)));
                    if (fabs(gradient) < M_PI_1_8) {
                        exist = true;
                        break;
                    }
                }
                
                if (!exist) neighbors.push_back(objects[i]);
                else continue;
            }
        }
    }
    
    return neighbors;
}

void TextDetector::groupingObjects(Text*& text, vector<Object*>objects)
{
    // 注目indexがオブジェクト数より大きくなるなら終了
    if (text->focusedIndex >= text->objects.size()) return;
    
    // 周辺オブジェクトのうち、条件を満たすものはグループに追加する
    addNeighbors(text, objects);
    
    // インクリメントして再帰
    text->focusedIndex++;
    groupingObjects(text, objects);
}

void TextDetector::addNeighbors(Text* text, vector<Object*> objects)
{
    double distance = 0, threshold = 0, gradient = 0;
    cv::Point diff, newVector, oldVector;
    
    // 注目オブジェクト取得
    Object* focus = text->objects[text->focusedIndex];
    
    for (int i = 0; i < objects.size(); i++) {
        
        if (focus->ID == objects[i]->ID) continue;
        if (text->contains(objects[i])) continue;
        
        // 閾値計算
        threshold = text->averaveDistance*1.5;
        
        // 距離計算
        distance = Distancer::distanceOfObjects(*focus, *objects[i]);//distanceOfCentroids(focus->centroid, objects[i]->centroid);
        
        // 閾値内判定
        if (distance < threshold) {
            
            // ベクトル勾配計算
            newVector = objects[i]->centroid - focus->centroid;
            oldVector = text->objects[text->originIndexes[text->focusedIndex]]->centroid - focus->centroid;
            gradient = acos(newVector.ddot(oldVector) / (pointSize(newVector)*pointSize(oldVector)));
            
            // 条件判定
            if (gradient < LOW_GRADIENT_THRESHOLD ||
                gradient > HIGH_GRADIENT_THRESHOLD)
            {
                // 追加
                // cout << threshold << endl;
                text->add(objects[i], distance);
                text->gradients.push_back(gradient);
                //Draw::drawText(srcImage, text);
            }
        }
    }
}


double TextDetector::computeGradient(Object obj1, Object obj2)
{
    cv::Point diff = obj1.centroid - obj2.centroid;
    return atan2(-diff.y, diff.x);
}

double TextDetector::pointSize(cv::Point p)
{
    return sqrt(p.x*p.x+p.y*p.y);
}