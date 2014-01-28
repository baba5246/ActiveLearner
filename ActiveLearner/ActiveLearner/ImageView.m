
#import "ImageView.h"

@implementation ImageView
{
    NSImageView *imageview;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        imageview = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 0, frame.size.width, frame.size.height)];
        imageview.imageScaling = NSImageScaleProportionallyUpOrDown;
        imageview.image = nil;
        [self addSubview:imageview];
    }
    return self;
}

- (void) setImage:(NSImage *)image
{
    imageview.image = image;
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
    // Drawing code here.
}

@end
