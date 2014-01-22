
#import "Controller.h"
#import "Processor.h"

@implementation Controller
{
    Processor *processor;
}

@synthesize imageView;

-(id) init
{
    self = [super init];
    if (self) {
        
        model = [Model sharedManager];
        processor = [Processor sharedManager];
        
        // Notification設定
        Notification *n = [Notification sharedManager];
        [n.nc addObserver:self selector:@selector(updatePathLabelDidLoad:) name:DID_LOAD_DIRECTORY object:nil];
        [n.nc addObserver:self selector:@selector(consoleMethod:) name:CONSOLE_OUTPUT object:nil];
        [n.nc addObserver:self selector:@selector(outputImage:) name:IMAGE_OUTPUT object:nil];
        
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


- (void) updatePathLabelDidLoad:(NSNotification*)n
{
    BOOL type = [n.userInfo[@"type"] boolValue];
    if (type) {
        [trainDirPathLbl setStringValue:model.trainDir];
    } else {
        [testDirPathLbl setStringValue:model.testDir];
    }
    [self console:@"読み込みが完了しました．"];
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

#pragma mark -
#pragma mark Image Output Methods

- (void) outputImage:(NSNotification *)n
{
    [imageView setImage:nil];
    NSData *data = n.userInfo[IMAGE_DATA];
    NSImage *image = [[NSImage alloc] initWithData:data];
    [imageView setImage:image];
}


@end
