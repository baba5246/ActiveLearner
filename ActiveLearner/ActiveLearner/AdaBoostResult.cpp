
#include "AdaBoostResult.h"

AdaBoostResult::AdaBoostResult(StrongClassifier& strong, WeakClassifier& selected)
{
    sc = strong;
    wc = selected;
}