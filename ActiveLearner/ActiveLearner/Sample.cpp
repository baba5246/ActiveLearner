
#include "Sample.h"

Sample::Sample(const Object& obj)
{
    object = obj;
    features = obj.features;
    filename = obj.filename;
}