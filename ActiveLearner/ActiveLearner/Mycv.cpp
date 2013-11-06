
#include "Mycv.h"

// コンストラクタ
Mycv::Mycv(const string&  filepath)
{
    src = imread(filepath, CV_LOAD_IMAGE_COLOR);
    //Draw draw(src);
}

//


