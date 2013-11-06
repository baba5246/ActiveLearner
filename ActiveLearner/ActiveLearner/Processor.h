
#import <Foundation/Foundation.h>
#import <opencv2/opencv.hpp>

@interface Processor : NSObject

+ (Processor*)sharedManager;

- (NSImage *)detectEdgesWithNSImage:(NSString*)filename;
- (NSImage*)detectControursFromFilename:(NSString*)filename;

- (NSImage*)NSImageFromIplImage:(IplImage*)iplImage;
- (IplImage*)IplImageFromNSImage:(NSImage*)nsimage;

@end
