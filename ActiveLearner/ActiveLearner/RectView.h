
#import <Cocoa/Cocoa.h>

@interface RectView : NSView

@property (nonatomic, retain) Truth *truth;
@property (nonatomic, copy) NSColor *color;
@property (nonatomic, assign) NSInteger number;

- (id) initWithFrame:(NSRect)frameRect rectNum:(NSInteger)rectNum;

- (void) saveTruth:(CGRect)option;

@end
