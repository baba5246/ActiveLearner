
#import <Foundation/Foundation.h>

@class Model;

@interface Controller : NSObject
{
    Model *model;
    
    IBOutlet id dirPathLbl;
    IBOutlet id fileNameLbl;
    IBOutlet id console;
    
    NSTableView *performTable;
}

@property (nonatomic, retain) IBOutlet ImgView *imgView;


#pragma mark -
#pragma mark Button Action Methods

//- (IBAction)onLeftButtonClicked:(id)sender;
//- (IBAction)onRightButtonClicked:(id)sender;

- (IBAction)onWholeClicked:(id)sender;
- (IBAction)onCCDClicked:(id)sender;
- (IBAction)onCCVClicked:(id)sender;
- (IBAction)onCGDClicked:(id)sender;
- (IBAction)onCGVClicked:(id)sender;

- (IBAction)onTestingClicked:(id)sender;


#pragma mark -
#pragma mark Observer Methods

-(void)observeValueForKeyPath:(NSString *)keyPath
                     ofObject:(id)object
                       change:(NSDictionary *)change
                      context:(void *)context;
@end
