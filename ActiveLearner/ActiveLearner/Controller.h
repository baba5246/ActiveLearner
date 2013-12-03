
#import <Foundation/Foundation.h>

@class Model;

@interface Controller : NSObject
{
    Model *model;
    
    IBOutlet id dirPathLbl;
    IBOutlet id fileNameLbl;
    IBOutlet id console;
    
    NSTableView *paramTable;
    NSTableView *performTable;
    NSTableView *correctRectsTable;
    NSTableView *roundCharaTable;
    
    IBOutlet NSComboBox *options;
}

@property (nonatomic, retain) IBOutlet ImgView *imgView;


#pragma mark -
#pragma mark Button Action Methods

-(IBAction)onLeftButtonClicked:(id)sender;
-(IBAction)onRightButtonClicked:(id)sender;
-(IBAction)onDoneButtonClicked:(id)sender;
-(IBAction)onSaveButtonClicked:(id)sender;

-(IBAction)startLearning:(id)sender;
-(IBAction)onStartLearningClicked:(id)sender;
-(IBAction)onComputePerformanceClicked:(id)sender;
-(IBAction)onSelectRectsClicked:(id)sender;
-(IBAction)onRoundCharactersClicked:(id)sender;


#pragma mark -
#pragma mark Observer Methods

-(void)observeValueForKeyPath:(NSString *)keyPath
                     ofObject:(id)object
                       change:(NSDictionary *)change
                      context:(void *)context;
@end
