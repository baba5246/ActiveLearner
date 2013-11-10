
#import <Foundation/Foundation.h>

@interface Processor : NSObject

+ (Processor*)sharedManager;

- (void) featuresFromImage:(NSString *)filepath;

- (NSImage *)detectEdgesWithNSImage:(NSString*)filename;
- (NSImage*)detectControursFromFilename:(NSString*)filename;

//- (NSImage*)NSImageFromIplImage:(IplImage*)iplImage;
//- (IplImage*)IplImageFromNSImage:(NSImage*)nsimage;

@end
