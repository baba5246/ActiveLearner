
#import <Foundation/Foundation.h>

@interface Processor : NSObject

+ (Processor*)sharedManager;

- (void) prepare;
- (void) makeSamples;
- (void) learnFeaturesWithAdaBoost;


@end
