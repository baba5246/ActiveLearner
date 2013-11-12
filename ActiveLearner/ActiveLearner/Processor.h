
#import <Foundation/Foundation.h>

@interface Processor : NSObject

+ (Processor*)sharedManager;

- (void) prepare;
- (void) featuresFromImage:(NSString *)filepath;


@end
