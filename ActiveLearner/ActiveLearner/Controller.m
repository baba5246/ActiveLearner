
#import "Controller.h"
#import "Processor.h"
#import "RectView.h"

@implementation Controller
{
    Processor *processor;
}

@synthesize imgView;

-(id) init
{
    self = [super init];
    if (self) {
        
        model = [Model sharedManager];
        processor = [Processor sharedManager];
        
        // Notification設定
        Notification *n = [Notification sharedManager];
        [n.nc addObserver:self selector:@selector(updateViewDidLoad:) name:DID_LOAD_DIRECTORY object:nil];
        [n.nc addObserver:self selector:@selector(showLoadErrorAlert) name:ERROR_LOAD_DIRECTORY object:nil];
        [n.nc addObserver:self selector:@selector(consoleMethod:) name:CONSOLE_OUTPUT object:nil];
        
    }
    return self;
}

- (IBAction)onTrainWholeClicked:(id)sender
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [processor trainWhole];
    });
}


- (IBAction)onTestWholeClicked:(id)sender
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [processor testWhole];
    });
}


#pragma mark -
#pragma mark Set View Methods


- (void) updateViewDidLoad:(NSNotification*)n
{
    BOOL type = [n.userInfo[@"type"] boolValue];
    if (type) {
        [trainDirPathLbl setStringValue:model.trainDir];
    } else {
        [testDirPathLbl setStringValue:model.testDir];
    }
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
