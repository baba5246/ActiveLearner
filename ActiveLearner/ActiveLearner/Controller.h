
#import <Foundation/Foundation.h>

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

@property (weak) IBOutlet NSImageView *imageView;


#pragma mark -
#pragma mark Button Action Methods

//- (IBAction)onLeftButtonClicked:(id)sender;
//- (IBAction)onRightButtonClicked:(id)sender;

- (IBAction)onTrainWholeClicked:(id)sender;

- (IBAction)onTestWholeClicked:(id)sender;


@end
