
#include "Text.h"

#pragma mark -
#pragma mark Inline Methods
inline double distanceObjects(Object*& obj1, Object*& obj2)
{
    Point diff = obj1->centroid - obj2->centroid;
    return sqrt(diff.x*diff.x+diff.y*diff.y);
}

inline double similarityColor(Scalar a, Scalar b)
{
    double dl = a.val[0]-b.val[0];
    double da = a.val[1]-b.val[1];
    double db = a.val[2]-b.val[2];
    return sqrt(dl*dl+da*da+db*db);
}

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
    computeObjectArea();
    computeTextArea();
}

void Text::countFalseObjects(vector<Object*>& objs)
{
    objectCount = objs.size();
    falseCount = 0;
    
    for (int i = 0; i < objs.size(); i++) {
        if (rect.contains(objs[i]->centroid)) {
            falseCount++;
        }
    }
    
    trueObjectRatio = 1 - (falseCount / (falseCount + objectCount));
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
    double echar = 0, fcorr = 0, gangle = 0, cr = 0;
    double sw = 0, aspect = 0, r = 0;
    
    long size = objects.size();
    for (int i = 0; i < size; i++) {
        echar += objects[i]->Echar;
        fcorr += objects[i]->Fcorr;
        gangle += objects[i]->Gangle;
        cr += objects[i]->CR;
        sw += objects[i]->strokeWidth;
        aspect += objects[i]->aspectRatio;
        r += objects[i]->r;
    }
    if (size>0) {
        aveEchar = echar / size;
        aveFcorr = fcorr / size;
        aveGangle = gangle / size;
        aveCR = cr / size;
        aveSW = sw / size;
        aveAspect = aspect / size;
        aveCircleR = r / size;
    }
}

void Text::computeVariantFeatures()
{
    long size = objects.size();
    double sw = 0, colorsim = 0, labsim = 0;
    for (int i = 0; i < size; i++) {
        // Stroke Width
        sw += pow((aveSW - objects[i]->strokeWidth), 2);
        
        for (int j = i+1; j < size; j++) {
            // Color Sim
            colorsim += similarityColor(objects[i]->color, objects[j]->color);
            labsim += similarityColor(objects[i]->labcolor, objects[j]->labcolor);
        }
    }
    double sizesize = (double)size*(size-1)*0.5f;
    if (size > 0) {
        varSW = sw / size;
        aveColorSim = colorsim / sizesize;
        aveLabSim = labsim / sizesize;
    } else {
        varSW = 0;
        aveColorSim = 0;
        aveLabSim = 0;
    }
    
    // Angle
    size = gradients.size();
    double angle = 0;
    for (int i = 0; i < size; i++) {
        angle += gradients[i];
    }
    if (size>0) {
        aveAngle = angle / size;
    }
    for (int i = 0; i < size; i++) {
        angle += pow((aveAngle - gradients[i]), 2);
    }
    if (size>0) {
        varAngle = angle / size;
    }
    
    
    // Distance
    size = distances.size();
    double dist = 0;
    for (int i = 0; i < size; i++) {
        dist += pow((aveDist - distances[i]), 2);
    }
    if (size>0) {
        varDist = dist / size;
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
    for (int y = 0; y < height+1; y++) {
        for (int x = 0; x < width+1; x++) {
            if (areaMap.at<int>(y, x) > 0) {
                objectArea++;
            }
        }
    }
    
    return objectArea;
}

double Text::computeTextArea()
{
    double r1, r2, d, x, a, b, h;
    Object *origin;
    
    long size = objects.size();
    for (int i = 1; i < size; i++) {
        
        origin = objects[originIndexes[i]];
        r1 = MIN(objects[i]->r, origin->r);
        r2 = MAX(objects[i]->r, origin->r);
        d = distanceObjects(objects[i], origin);
        x = r1 * d / (r2 - r1);
        a = r1 * (x + r1) / x;
        b = r2 * (x + r1 + r2 + d) / x;
        h = sqrt(pow((r1+r2+d), 2) - pow((b-a), 2));
        textArea += (a + b) * h * 0.5;
        
    }
    
    return textArea;
}

