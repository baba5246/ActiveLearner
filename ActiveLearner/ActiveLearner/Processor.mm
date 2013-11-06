
#import "Processor.h"

@implementation Processor

static Processor* sharedProcessor = nil;

+ (Processor*)sharedManager {
    @synchronized(self) {
        if (sharedProcessor == nil) {
            sharedProcessor = [[self alloc] init];
        }
    }
    return sharedProcessor;
}

- (NSImage *)detectEdgesFromFilename:(NSString*)filename
{
    IplImage *image = cvLoadImage([filename UTF8String]);
    
    if (image == NULL)
    {
        NSLog(@"Image is null!");
        return nil;
    }
    
    IplImage *edges = cvCreateImage(cvGetSize(image), 8, 1);
    [self edgeDetectorWithSrc:image dst:edges];
    
    NSImage *nsimage = [self NSImageFromIplImage:edges];
    return nsimage;
}

- (void) edgeDetectorWithSrc:(IplImage *)src dst:(IplImage*)dst
{
    IplImage *gray = cvCreateImage(cvGetSize(src), 8, 1);
    
    cvCvtColor(src, gray, CV_RGB2GRAY);
    cvCanny(gray, dst, 50, 200);
    
    cvSaveImage("edge.png", dst);
}

- (NSImage*)detectControursFromFilename:(NSString*)filename
{
    IplImage *image = cvLoadImage([filename UTF8String]);
    
    if (image == NULL)
    {
        NSLog(@"Image is null!");
        return nil;
    }
    
    IplImage *contours = cvCreateImage(cvGetSize(image), image->depth, 3);
    [self controursDetectorWithSrc:image dst:contours];
    
    NSImage *nsimage = [self NSImageFromIplImage:contours];
    return nsimage;

}

- (void) controursDetectorWithSrc:(IplImage*)src dst:(IplImage*)dst
{
    IplImage *edges = cvCreateImage(cvGetSize(src), 8, 1);
    [self edgeDetectorWithSrc:src dst:edges];
    
    CvMemStorage *storage = cvCreateMemStorage(0);
    CvSeq *contours = 0;
    cvFindContours(edges, storage, &contours);
    
    for (; contours != 0; contours = contours->h_next)
    {
        cvDrawContours(dst, contours, CV_RGB(rand()&255, rand()&255, rand()&255),
                       CV_RGB(0,0,0), -1, CV_FILLED, 8, cvPoint(0,0));
    }
    
    cvSaveImage("contours.png", dst);
}

- (NSImage*)NSImageFromIplImage:(IplImage*)iplImage
{
    
    char *d = iplImage->imageData; // Get a pointer to the IplImage image data.
        
    NSString *COLORSPACE;
    if(iplImage->nChannels == 1){
        COLORSPACE = NSDeviceWhiteColorSpace;
    }
    else{
        COLORSPACE = NSDeviceRGBColorSpace;
    }
    
    NSBitmapImageRep *bmp = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                    pixelsWide:iplImage->width
                                                                    pixelsHigh:iplImage->height
                                                                 bitsPerSample:iplImage->depth
                                                               samplesPerPixel:iplImage->nChannels
                                                                      hasAlpha:NO
                                                                      isPlanar:NO
                                                                colorSpaceName:COLORSPACE
                                                                   bytesPerRow:iplImage->widthStep bitsPerPixel:0];
    
    int x, y;
    unsigned long colors[3];
    for(y = 0; y < iplImage->height; y++){
        for(x = 0; x < iplImage->width; x++){
            if(iplImage->nChannels > 1){
                colors[2] = (unsigned long) d[(y * iplImage->widthStep) + (x*3)];
                colors[1] = (unsigned long) d[(y * iplImage->widthStep) + (x*3)+1];
                colors[0] = (unsigned long) d[(y * iplImage->widthStep) + (x*3)+2];
            }
            else{
                colors[0] = (unsigned long)d[(y * iplImage->width) + x];
            }
            [bmp setPixel:(unsigned long*)colors atX:x y:y];
        }
    }
    
    NSData *tif = [bmp TIFFRepresentation];
    NSImage *im = [[NSImage alloc] initWithData:tif];
        
    return im;
    
}

- (IplImage*)IplImageFromNSImage:(NSImage*)nsimage
{
    CGImageRef cgimage = [self cgimageRefFromNSImage:nsimage size:[nsimage size]];
    
    CGContextRef context;
    CGColorSpaceRef colorSpace;
    IplImage *iplImageTemp, *iplImage;
    
    // RGB色空間を作成
    colorSpace = CGColorSpaceCreateDeviceRGB();
    
    // 一時的なIplImageを作成
    iplImageTemp = cvCreateImage(cvSize([nsimage size].width, [nsimage size].height), IPL_DEPTH_8U, 4);
    
    // CGBitmapContextをIplImageのビットマップデータのポインタから作成
    context = CGBitmapContextCreate(iplImageTemp->imageData,
                                    iplImageTemp->width,
                                    iplImageTemp->height,
                                    iplImageTemp->depth,
                                    iplImageTemp->widthStep,
                                    colorSpace,
                                    kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault);
    
    // CGImageをCGBitmapContextに描画
    CGContextDrawImage(context, CGRectMake(0.0f, 0.0f, [nsimage size].width, [nsimage size].height), cgimage);
    
    // ビットマップコンテキストと色空間を解放
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    
    // 最終的なIplImageを作成
    iplImage = cvCreateImage(cvGetSize(iplImageTemp), IPL_DEPTH_8U, 3);
    cvCvtColor(iplImageTemp, iplImage, CV_RGBA2RGB);
    
    // 一時的なIplImageを解放
    cvReleaseImage(&iplImageTemp);
    
    return iplImage;
}

- (CGImageRef) cgimageRefFromNSImage:(NSImage *)image size:(CGSize)size
{
    NSUInteger canvasWidth, canvasHeight;
    CGFloat imageWidth, imageHeight;
    CGFloat imageAspect, canvasAspect;
    CGFloat targetLeft;
        
    imageWidth = [image size].width;
    imageHeight = [image size].height;
    if(imageWidth <= 0 || imageHeight <= 0 || size.width <= 0 || size.height <= 0) return NULL;
    
    imageAspect = imageWidth / imageHeight;
    canvasAspect = size.width / size.height;
    if(imageAspect > canvasAspect) {
        canvasWidth = imageWidth;
        canvasHeight = imageWidth / canvasAspect;
        targetLeft = 0;
    } else {
        canvasHeight = imageHeight;
        canvasWidth = imageHeight * canvasAspect;
        targetLeft = (canvasWidth - imageWidth) / 2.0;
    }
    
    size_t bytesPerRow = 4*canvasWidth;
    void* bitmapData = malloc(bytesPerRow * canvasHeight);
    CGContextRef context = CGBitmapContextCreate(bitmapData,
                                                    canvasWidth,
                                                    canvasHeight,
                                                    8,
                                                    bytesPerRow,
                                                    [[NSColorSpace genericRGBColorSpace] CGColorSpace],
                                                    kCGBitmapByteOrder32Host|kCGImageAlphaPremultipliedFirst);
    
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:context flipped:NO]];
    // 透明色で塗りつぶす
    [[NSColor clearColor] set];
    NSRectFill(NSMakeRect(0, 0, canvasWidth, canvasHeight));
    // 下揃え中央揃えで画像を描画
    [image drawInRect:NSMakeRect(targetLeft,0, imageWidth, imageHeight) fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
    [NSGraphicsContext restoreGraphicsState];
    
    CGImageRef cgImage = CGBitmapContextCreateImage(context);
    CGContextRelease(context);
    free(bitmapData);
        
    return cgImage;
}


@end
