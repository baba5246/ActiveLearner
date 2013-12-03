
#import <Foundation/Foundation.h>

@interface Notification : NSObject

@property (nonatomic, retain) NSNotificationCenter *nc;
@property (nonatomic, retain) NSNotificationCenter *animation;
@property (nonatomic, retain) NSNotificationCenter *server;

+ (Notification*)sharedManager;

- (void) sendNotification:(NSString *)name;
- (void) sendNotification:(NSString *)name objectsAndKeys:(id)object, ...NS_REQUIRES_NIL_TERMINATION;
- (void) sendAnimationNotification:(NSString *)name;
- (void) sendAnimationNotification:(NSString *)name objectsAndKeys:(id)object, ...NS_REQUIRES_NIL_TERMINATION;
- (void) sendServerNotification:(NSString *)name;
- (void) sendServerNotification:(NSString *)name objectsAndKeys:(id)object, ...NS_REQUIRES_NIL_TERMINATION;
- (void) removeObserver:(NSObject *)observer;

@end
