
#import "ImgView.h"
#import "RectView.h"

@implementation ImgView
{
    Model *model;
    NSPoint start, previous, moving, end;
    
    RectView *rectView;
    InputView *input;
}

@synthesize rectNum;

- (void)prepare
{
    model = [Model sharedManager];
    
    [model addObserver:self forKeyPath:RECTANGLES_KEY
               options:(NSKeyValueObservingOptionNew|NSKeyValueObservingOptionPrior)
               context:nil];
    
    start = NSMakePoint(-1, -1);
}

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor clearColor] setFill];
    NSRectFill(dirtyRect);
    [super drawRect:dirtyRect];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if (model.imagePath.path.length>0) {
        start = [self convertPoint:[theEvent locationInWindow] fromView:[[self window] contentView]];
    } else {
        return;
    }
    
    if (rectView != nil) {
        [rectView removeFromSuperview];
        [input removeFromSuperview];
    }
    
    rectNum++;
    NSRect rect = NSMakeRect(start.x, start.y, start.x-start.x, start.y-start.y);
    rectView = [[RectView alloc] initWithFrame:rect rectNum:rectNum];
    [self addSubview:rectView];
}

-(void)mouseDragged:(NSEvent *)theEvent
{
    if (start.x == 0 && start.y == 0) return;
    
    moving = [self convertPoint:[theEvent locationInWindow] fromView:[[self window] contentView]];
    
    float ox, oy;

    if (start.x > moving.x) ox = moving.x;
    else ox = start.x;
    
    if (start.y > moving.y) oy = moving.y;
    else oy = start.y;
    
    [rectView setFrame:NSMakeRect(ox, oy, fabs(start.x-moving.x), fabs(start.y-moving.y))];
    [rectView setNeedsDisplay:YES];

}

- (void)mouseUp:(NSEvent *)theEvent
{
    if (start.x == 0 && start.y == 0) return;
    
    end = [self convertPoint:[theEvent locationInWindow] fromView:[[self window] contentView]];
    
    [self addInputView:start];
    //[rectView saveTruth];
}


- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    if ([keyPath isEqual:RECTANGLES_KEY])
    {
        if ([[model getRectangles] count] == 0) [self resetRectangles];
    }
    else if ([keyPath isEqual:@""])
    {
        
    }
    
}

- (void) changeRectanglesState
{
    NSArray *subviews = self.subviews.copy;
    for (RectView *view in subviews) {
        view.color = [NSColor greenColor];
        [view setNeedsDisplay:YES];
    }
}

- (void) resetRectangles
{
    NSArray *subviews = self.subviews.copy;
    for (RectView *view in subviews) {
        [view removeFromSuperview];
    }
}

- (void) addInputView:(NSPoint) origin
{
    if (input != nil) [input removeFromSuperview];
        
    input = [[InputView alloc] initWithOrigin:origin];
    input.delegate = self;
    [self addSubview:input];
}

- (void) endEditing
{
    rectView.truth.text = input.stringValue;
    CGRect option;
    if (self.image.size.width > self.image.size.height) option = CGRectMake(0, 80, self.image.size.width, self.image.size.height);
    else option = CGRectMake(80, 0, self.image.size.width, self.image.size.height);
    [rectView saveTruth:option];
    
    rectView = nil;
}

@end
