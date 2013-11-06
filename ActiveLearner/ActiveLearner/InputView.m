
#import "InputView.h"

@implementation InputView

@synthesize delegate;

- (id)initWithOrigin:(NSPoint)origin
{
    self = [super initWithFrame:NSMakeRect(0, 0, 120, 20)];
    if (self) {
        [self setFrameOrigin:origin];
    }
    
    return self;
}

- (void)textDidEndEditing:(NSNotification *)notification
{
    [delegate endEditing];
    [self removeFromSuperview];
}

@end
