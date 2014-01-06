
#import <Foundation/Foundation.h>

@interface Processor : NSObject

+ (Processor*)sharedManager;

- (void) prepare;

- (void) excuteWhole:(BOOL)type;
//- (void) debugCCD;
//- (void) debugCCV;
//- (void) debugCGD;
//- (void) debugCGV;


@end
