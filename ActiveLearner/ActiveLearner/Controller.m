
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
        [n.nc addObserver:self selector:@selector(updateViewDidLoad) name:DID_LOAD_DIRECTORY object:nil];
        [n.nc addObserver:self selector:@selector(showLoadErrorAlert) name:ERROR_LOAD_DIRECTORY object:nil];
        
    }
    return self;
}

- (IBAction)startLearning:(id)sender
{
    // show progress
    [self console:@"特徴量の抽出中... "];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        // make samples
        [processor makeSamples];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [self console:@"特徴量の抽出完了！学習開始！"];
        });
        
        // AdaBoost Learning
        [processor learnFeaturesWithAdaBoost];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [self console:@"学習終了！"];
        });
    });
    
    
}

#pragma mark -
#pragma mark Set View Methods


- (void) updateViewDidLoad
{
    [dirPathLbl setStringValue:model.directory];
    [self console:@"読み込みが完了しました."];
    [self console:@"あ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\nあ\n"];
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

- (void) console:(NSString *) output
{
    [console string];
    
    NSTextStorage *con = [console textStorage];
    NSString *text = [[console textContainer].textView string];
    text = [text stringByAppendingFormat:@"%@\n", output];
    [console setStringValue:text];
}


@end
