
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
        [model addObserver:self forKeyPath:IMAGE_PATH_KEY
                   options:(NSKeyValueObservingOptionNew|NSKeyValueObservingOptionPrior)
                   context:nil];
        [model addObserver:self forKeyPath:RECTANGLES_KEY
                   options:(NSKeyValueObservingOptionNew|NSKeyValueObservingOptionPrior)
                   context:nil];
        
        processor = [Processor sharedManager];
    }
    return self;
}

- (IBAction)startLearning:(id)sender
{
    // show progress
    [desctiptionLbl setStringValue:@"特徴量の抽出中... "];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        // make samples
        [processor makeSamples];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [desctiptionLbl setStringValue:@"特徴量の抽出完了！学習開始！"];
        });
        
        // AdaBoost Learning
        [processor learnFeaturesWithAdaBoost];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [desctiptionLbl setStringValue:@"学習終了！"];
        });
    });
    
    
}


#pragma mark -
#pragma mark Button Action Methods

-(IBAction)onLeftButtonClicked:(id)sender
{
    if (model.fileIndex > 0 && model.files.count > 0)
    {
        // アラート表示
        if ([[model getRectangles] count] > 0)
        {
            NSAlert *alert = [self deleteRectangleAlertView];
            if ([alert runModal] == NSAlertFirstButtonReturn) // OKボタン
            {
                // rectanglesを消す
                [model resetRectangles];
                
                // 前の画像の準備
                [model setPreviousFileInfo];
                
                // 画像をセット
                [self setImageFromFilePath];
            }
        } else {
            // rectanglesを消す
            [model resetRectangles];
            
            // 前の画像の準備
            [model setPreviousFileInfo];
            
            // 画像をセット
            [self setImageFromFilePath];
        }
        

    }
}

-(IBAction)onRightButtonClicked:(id)sender
{
    if (model.fileIndex < [model.files count]-1 && model.files.count > 0)
    {
        // アラート表示
        if ([[model getRectangles] count] > 0) {
            NSAlert *alert = [self deleteRectangleAlertView];
            if ([alert runModal] == NSAlertFirstButtonReturn) // OKボタン
            {
                // rectanglesを消す
                [model resetRectangles];
                
                // 次の画像の準備
                [model setNextFileInfo];
                
                // 画像をセット
                [self setImageFromFilePath];
            }
        }
        else
        {
            // rectanglesを消す
            [model resetRectangles];
            
            // 次の画像の準備
            [model setNextFileInfo];
            
            // 画像をセット
            [self setImageFromFilePath];
        }
    }
}

- (IBAction)onDoneButtonClicked:(id)sender
{
    NSInteger index = [options indexOfSelectedItem];
    NSString *item = [options itemObjectValueAtIndex:index];
    NSLog(@"selected index: %ld, item:%@", (long)index, item);
    
    //[self imageProcessing:index];
}


-(IBAction)onSaveButtonClicked:(id)sender
{
    model = [Model sharedManager];
    
    if ([model saveRectangles])
    {
        [imgView changeRectanglesState];
        [xmlDataCountLabel setIntegerValue:[[model.getXMLData allKeys] count]];
        
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"OK"];
        [alert setMessageText:@"Succeeded!"];
        [alert setInformativeText:@"Saving operation finished successfully!"];
        [alert setAlertStyle:NSWarningAlertStyle];
        
        [alert runModal];
    }
    else
    {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"OK"];
        [alert setMessageText:@"Failed!"];
        [alert setInformativeText:@"Saving operation was failed because there are no rectangles..."];
        [alert setAlertStyle:NSWarningAlertStyle];
        
        [alert runModal];
    }
}


#pragma mark -
#pragma mark Observer Methods

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    if ([keyPath isEqual:IMAGE_PATH_KEY])
    {
        // imgViewの準備
        [imgView prepare];
        
        // 画像をセット
        [self setImageFromFilePath];
    }
    else if ([keyPath isEqual:RECTANGLES_KEY])
    {
        [rectCountLabel setIntegerValue:[[model getRectangles] count]];
    }
    else if ([keyPath isEqual:@""])
    {
        
    
    }
    
}


#pragma mark -
#pragma mark Set View Methods

- (void) setImageFromFilePath
{
    if (model.filename.length>0) [fileNameLabel setStringValue:model.filename];
    
    NSImage *image = [[NSImage alloc] initWithContentsOfURL:model.imagePath];
    imgView.image = image;
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

- (NSAlert *) progressAlertView
{
    
}

#pragma mark -
#pragma mark Image Processing Methods




@end
