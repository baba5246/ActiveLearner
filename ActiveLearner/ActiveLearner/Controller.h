
#import <Foundation/Foundation.h>
#import "ImageView.h"

@class Model;

@interface Controller : NSObject
{
    Model *model;
    
    IBOutlet id trainDirPathLbl;
    IBOutlet id testDirPathLbl;
    IBOutlet id fileNameLbl;
    IBOutlet id console;
    
    NSTableView *performTable;
}

@property (unsafe_unretained) IBOutlet NSTextView *consoleView;
@property (weak) IBOutlet NSScrollView *textScrollView;
@property (weak) IBOutlet NSTextField *imageNameLbl;
@property (weak) IBOutlet ImageView *imageView;


#pragma mark -
#pragma mark Button Action Methods

//- (IBAction)onLeftButtonClicked:(id)sender;
//- (IBAction)onRightButtonClicked:(id)sender;

- (IBAction)onTrainWholeClicked:(id)sender;

- (IBAction)onTestWholeClicked:(id)sender;


@end
