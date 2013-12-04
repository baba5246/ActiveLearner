
#include "Sample.h"

Sample::Sample(const Object& obj)
{
    object = obj;
    features = obj.features;
    filename = obj.filename;
}

Sample::Sample(const Text& t)
{
    text = t;
    features = t.features;
    filename = t.filename;
}