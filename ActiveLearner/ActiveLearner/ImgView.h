
#import <Cocoa/Cocoa.h>

@interface ImgView : NSImageView <InputViewDelegate>


@property (nonatomic, assign) NSInteger rectNum;

- (void) prepare;

- (void) changeRectanglesState;

- (void) observeValueForKeyPath:(NSString *)keyPath
                       ofObject:(id)object
                         change:(NSDictionary *)change
                        context:(void *)context;
@end
