
#include "Mycv.h"

// コンストラクタ
Mycv::Mycv(const string&  filepath)
{
    Mat image;
    image = imread(filepath, CV_LOAD_IMAGE_COLOR);
    Draw draw(image);
}