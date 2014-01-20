
#import <Foundation/Foundation.h>
#import "AdaBoost.h"

@interface Processor : NSObject
{
    Model *model;
    Notification *n;
    
    NSDictionary *xmldata;
    
    AdaBoost ccvAdaBoost;
    AdaBoost cgvAdaBoost;
    
}

+ (Processor*)sharedManager;

- (void) prepare;

- (void) trainWhole;
- (void) testWhole;

@end
