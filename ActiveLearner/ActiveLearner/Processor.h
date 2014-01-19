
#import <Foundation/Foundation.h>

@interface Processor : NSObject

+ (Processor*)sharedManager;

- (void) prepare;

- (void) trainWhole;
- (void) testWhole;

@end
