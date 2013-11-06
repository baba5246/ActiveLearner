
#import "RectView.h"

@implementation RectView

@synthesize number, truth, color;

- (id) initWithFrame:(NSRect)frameRect rectNum:(NSInteger)rectNum
{
    self = [super initWithFrame:frameRect];
    if (self) {
        number = rectNum;
        truth = [[Truth alloc] init];
        color = [NSColor redColor];
    }
    return self;
}

- (void) drawRect:(NSRect)dirtyRect
{
    NSRect rect = self.bounds;
    [color set];
    NSFrameRect(rect);
}

- (void) saveTruth:(CGRect)option
{
    Model *model = [Model sharedManager];
    int originX = self.frame.origin.x-10-option.origin.x;
    int originY = 650 - (self.frame.origin.y + self.frame.size.height)-option.origin.y;
    CGRect adjusted = CGRectMake(originX, originY, (int)self.frame.size.width, (int)self.frame.size.height);
    truth.rect = adjusted;
    [model addTruth:truth];
}

@end
