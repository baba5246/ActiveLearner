
#import "Controller.h"
#import "Processor.h"
#import "RectView.h"

@implementation Controller
{
    Processor *processor;
    
    int count;
}

@synthesize imgView;

-(id) init
{
    self = [super init];
    if (self) {
        
        model = [Model sharedManager];
        processor = [Processor sharedManager];
        
        count = 0;
        
        // Notification設定
        Notification *n = [Notification sharedManager];
        [n.nc addObserver:self selector:@selector(updateViewDidLoad) name:DID_LOAD_DIRECTORY object:nil];
        [n.nc addObserver:self selector:@selector(showLoadErrorAlert) name:ERROR_LOAD_DIRECTORY object:nil];
        [n.nc addObserver:self selector:@selector(consoleMethod:) name:CONSOLE_OUTPUT object:nil];
        
    }
    return self;
}

- (IBAction)onWholeClicked:(id)sender
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [processor excuteWhole:TYPE_TRAINING];
    });
    
}

- (IBAction)onCCDClicked:(id)sender
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
      
        //[processor debugCCD];
        
    });
}

- (IBAction)onCCVClicked:(id)sender
{
    
}

- (IBAction)onCGDClicked:(id)sender
{
    
}

- (IBAction)onCGVClicked:(id)sender
{
    
}

- (IBAction)onTestingClicked:(id)sender
{
    
}


#pragma mark -
#pragma mark Set View Methods


- (void) updateViewDidLoad
{
    [dirPathLbl setStringValue:model.directory];
    [self console:@"読み込みが完了しました．"];
}

- (NSAlert *) deleteRectangleAlertView
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert addButtonWithTitle:@"Cancel"];
    [alert setMessageText:@"Delete rectangles information?"];
    [alert setInformativeText:@"Push the \"Save\" button for saving the info of rectangles."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    return alert;
}

- (void) showLoadErrorAlert
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Loading Error!"];
    [alert setInformativeText:@"Select a directory."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    if ([alert runModal] == NSAlertFirstButtonReturn) // OKボタン
    {
        
    }
}


#pragma mark -
#pragma mark Console Writing Methods

- (void) consoleMethod:(NSNotification *) n
{
    NSString *output = [n userInfo][@"output"];
    [self console:output];
}

- (void) console:(NSString *) output
{
//    NSString *text = [NSString stringWithFormat:@"%@\n", output];
//    [[console textStorage] appendAttributedString:[[NSAttributedString alloc] initWithString:text]];
//
////    NSRange range = NSMakeRange([[console string] length], 0);
//    [console scrollToBeginningOfDocument:[console string]];
//    [console scrollToEndOfDocument:[console string]];
}


@end
