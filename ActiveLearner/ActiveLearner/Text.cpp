
#include "Text.h"

#pragma mark -
#pragma mark Contructors

Text::Text()
{
    
}

Text::Text(string path, Object*& obj)
{
    filepath = path;
    filename = filepath.substr(filepath.find_last_of("/")+1);
    srcImage = imread(filepath);
    srcW = srcImage.cols;
    srcH = srcImage.rows;
    add(obj, 0);
}


#pragma mark -
#pragma mark Public Methods

void Text::add(Object*& obj, double distance)
{
    for (int i = 0; i < objects.size(); i++) {
        if (!objects[i]->ID.compare(obj->ID)) {
            return;
        }
    }
    
    objects.push_back(obj);
    originIndexes.push_back(focusedIndex);
    computeColor();
    computeStrokeWidth();
    if (objects.size()>1) {
        distances.push_back(distance); //　オブジェクト数が2以上になったらdistanceにadd
        computeAverageDistance();
    }
}

void Text::add(Text *&text)
{
    vector<Object*> temp_objects;
    for (int i = 0; i < objects.size(); i++)
        for (int j = 0; j < text->objects.size(); j++) {
            if (!objects[i]->ID.compare(text->objects[j]->ID)) {
                temp_objects.push_back(text->objects[j]);
            }
        }
    
    objects.insert(objects.end(), temp_objects.begin(), temp_objects.end());
    aveDist += text->aveDist;
    aveDist /= 2.0f;
    computeColor();
    computeStrokeWidth();
    computeRatioFeatures();
}

bool Text::areAllFocused()
{
    for (int i = 0; i < objects.size(); i++) {
        if (!objects[i]->didFocused) return false;
    }
    
    return true;
}

bool Text::contains(Object*& obj)
{
    for (int i = 0; i < objects.size(); i++) {
        if (!obj->ID.compare(objects[i]->ID)) return true;
    }
    
    return false;
}


#pragma mark -
#pragma mark Private Methods

void Text::computeProperties()
{
    computeAverageFeatures();
    computeVariantFeatures();
    computeRatioFeatures();
}

void Text::computeColor()
{
    double r = 0, g = 0, b = 0;
    long length = objects.size();
    
    if (length != 0) {
        for (int i = 0; i < length; i++) {
            r += objects[i]->color[0];
            g += objects[i]->color[1];
            b += objects[i]->color[2];
        }
        r /= length;
        g /= length;
        b /= length;
    }
    
    color = Scalar(r, g, b);
}

void Text::computeAverageDistance()
{
    double dist = 0;
    long size = distances.size();
    if (size == 0) return;
    
    for (int i = 0; i < size; i++) {
        dist += distances[i];
    }
    if (size>0) aveDist = dist / size;
    else aveDist = 0;
}

void Text::computeStrokeWidth()
{
    double tempsw = 0;
    long size = objects.size();
    for (int i = 0; i < size; i++) {
        tempsw += objects[i]->strokeWidth;
    }
    if (size>0) tempsw /= size;
    aveSW = tempsw;
}


void Text::computeAverageFeatures()
{
    long size = objects.size();
    double Echar = 0, Fcorr = 0, Gangle = 0, CR = 0;
    for (int i = 0; i < size; i++) {
        Echar += objects[i]->Echar;
        Fcorr += objects[i]->Fcorr;
        Gangle += objects[i]->Gangle;
        CR += objects[i]->CR;
    }
    if (size>0) {
        aveEchar = Echar / size;
        aveFcorr = Fcorr / size;
        aveGangle = Gangle / size;
        aveCR = CR / size;
    }
}

void Text::computeVariantFeatures()
{
    long size = objects.size();
    double diffSW = 0, diffR = 0, diffG = 0, diffB = 0;
    for (int i = 0; i < size; i++) {
        
        for (int j = i+1; j < size; j++) {
            
            // Stroke Width
            diffSW += fabs(objects[i]->strokeWidth - objects[j]->strokeWidth);
            
            // Color
            diffR += fabs(objects[i]->color[0] - objects[j]->color[0]);
            diffG += fabs(objects[i]->color[1] - objects[j]->color[1]);
            diffB += fabs(objects[i]->color[2] - objects[j]->color[2]);
        }
    }
    double sizesize = (double)size*(size-1)*0.5f;
    if (size > 0) {
        varSW = diffSW / sizesize;
        varColorR = diffR / sizesize;
        varColorG = diffG / sizesize;
        varColorB = diffB / sizesize;
    } else {
        varSW = 0;
        varColorR = 0;
        varColorG = 0;
        varColorB = 0;
    }
    
    // Angle
    size = gradients.size();
    double diffGrad = 0;
    for (int i = 0; i < size; i++) {
        if (i < size-1) {
            diffGrad += fabs(gradients[i] - gradients[i+1]);
        }
    }
    if (size>0) {
        varAngle = diffGrad / size;
    } else {
        varAngle = 0;
    }

    
    // Distance
    size = distances.size();
    double diffDist = 0;
    for (int i = 0; i < size; i++) {
        if (i < size-1) {
            diffDist += fabs(distances[i] - distances[i+1]);
        }
    }
    if (size>0) {
        varDist = diffDist / size;
    } else {
        varDist = 0;
    }
}


void Text::computeRatioFeatures()
{
    int minx = INFINITY, miny = INFINITY, maxx = 0, maxy = 0;
    for (int i = 0; i < objects.size(); i++)
    {
        if (minx > objects[i]->rect.tl().x) minx = objects[i]->rect.tl().x;
        if (miny > objects[i]->rect.tl().y) miny = objects[i]->rect.tl().y;
        if (maxx < objects[i]->rect.br().x) maxx = objects[i]->rect.br().x;
        if (maxy < objects[i]->rect.br().y) maxy = objects[i]->rect.br().y;
    }
    
    width = maxx - minx + 1;
    height = maxy - miny + 1;
    rect = cv::Rect(minx, miny, width, height);
    rectRatio = (double)rect.area() / (srcImage.rows*srcImage.cols);
    aspectRatio = (double) MIN(width, height) / MAX(width, height);
    if (width > height) longLengthRatio = (double)width / srcImage.cols;
    else longLengthRatio = (double)height / srcImage.rows;
    
    double objArea = computeObjectArea();
    objAreaRatio = objArea / rect.area();
}

double Text::computeObjectArea()
{
    // いもす法
    Mat_<int> areaMap(height+1, width+1);
    areaMap = 0;
    for (int i = 0; i < objects.size(); i++)
    {
        Point tl = objects[i]->rect.tl(), br = objects[i]->rect.br();
        Point first = tl -rect.tl();
        Point second = br+Point(1,1)-rect.tl();
        Point third = Point(tl.x, br.y+1) - rect.tl();
        Point fourth = Point(br.x+1, tl.y) - rect.tl();
        areaMap.at<int>(first) += 1;
        areaMap.at<int>(second) += 1;
        areaMap.at<int>(third) -= 1;
        areaMap.at<int>(fourth) -= 1;
    }
    for (int y = 0; y < height+1; y++) {
        for (int x = 1; x < width+1; x++) {
            areaMap.at<int>(y, x) += areaMap.at<int>(y, x-1);
        }
    }
    for (int y = 1; y < height+1; y++) {
        for (int x = 0; x < width+1; x++) {
            areaMap.at<int>(y, x) += areaMap.at<int>(y-1, x);
        }
    }
    double objectArea = 0;
//    Mat dst = Mat::zeros(srcH, srcW, CV_8UC3);
    for (int y = 0; y < height+1; y++) {
        for (int x = 0; x < width+1; x++) {
            if (areaMap.at<int>(y, x) > 0) {
//                dst.at<Vec3b>(y+rect.tl().y, x+rect.tl().x)[0] = (unsigned char)BRIGHTNESS;
//                dst.at<Vec3b>(y+rect.tl().y, x+rect.tl().x)[1] = (unsigned char)BRIGHTNESS;
//                dst.at<Vec3b>(y+rect.tl().y, x+rect.tl().x)[2] = (unsigned char)BRIGHTNESS;
                objectArea++;
            }
        }
    }
    
    return objectArea;
}

