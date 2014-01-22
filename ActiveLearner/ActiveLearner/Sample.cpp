
#include "Sample.h"

Sample::Sample(Object*& obj)
{
    object = obj;
    features = obj->features;
    filename = obj->filename;
    hasObject = true;
}

Sample::Sample(Text*& t)
{
    text = t;
    features = t->features;
    filename = t->filename;
    hasObject = false;
}