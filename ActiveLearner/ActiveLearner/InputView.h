
#import <Cocoa/Cocoa.h>

@class InputView;

@protocol InputViewDelegate <NSObject>

- (void) endEditing;

@end

@interface InputView : NSTextField
{
    id<InputViewDelegate> delegate;
}

@property (nonatomic, retain) id<InputViewDelegate> delegate;

- (id)initWithOrigin:(NSPoint)origin;

@end
