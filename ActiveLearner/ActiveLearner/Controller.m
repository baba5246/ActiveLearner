
#import "Controller.h"
#import "Processor.h"

@implementation Controller
{
    Processor *processor;
}

@synthesize consoleView, textScrollView;
@synthesize imageView, imageNameLbl;

-(id) init
{
    self = [super init];
    if (self) {
        
        model = [Model sharedManager];
        processor = [Processor sharedManager];
        
        [consoleView setHorizontallyResizable:YES];
        [consoleView setVerticallyResizable:YES];
        
        // Notification設定
        Notification *n = [Notification sharedManager];
        [n.nc addObserver:self selector:@selector(updatePathLabelDidLoad:) name:DID_LOAD_DIRECTORY object:nil];
        [n.nc addObserver:self selector:@selector(consoleMethod:) name:CONSOLE_OUTPUT object:nil];
        [n.nc addObserver:self selector:@selector(outputImage:) name:IMAGE_OUTPUT object:nil];
        [n.nc addObserver:self selector:@selector(updateImagePathLabel:) name:UPDATE_IMAGE_NAME object:nil];
        
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

- (void) updateImagePathLabel:(NSNotification*)n
{
    NSString *path = n.userInfo[FILEPATH];
    [imageNameLbl setStringValue:path];
}


#pragma mark -
#pragma mark Console Writing Methods

- (void) consoleMethod:(NSNotification *) n
{
    NSString *output = [n userInfo][OUTPUT];
    [self console:output];
}

- (void) console:(NSString *) output
{
    LOG(@"%@", output);
//    NSRange insertAtEnd=NSMakeRange([[consoleView textStorage] length],0);
//    [consoleView replaceCharactersInRange:insertAtEnd
//                          withString:output];
//    [textScrollView setScrollsDynamically:YES];
}


#pragma mark -
#pragma mark Image Output Methods

- (void) outputImage:(NSNotification *)n
{
    NSData *data = nil;
    NSImage *image = nil;
    [imageView setImage:nil];
    
    data = n.userInfo[IMAGE_DATA];
    image = [[NSImage alloc] initWithData:data];
    [imageView setImage:image];
}


@end
