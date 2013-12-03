
#import "Notification.h"

@implementation Notification

@synthesize nc, animation, server;

static Notification* sharedNotification = nil;

+ (Notification*)sharedManager {
    @synchronized(self) {
        if (sharedNotification == nil) {
            sharedNotification = [[self alloc] init];
            [sharedNotification prepare];
        }
    }
    return sharedNotification;
}

- (void) prepare
{
    nc = [NSNotificationCenter defaultCenter];
    animation = [[NSNotificationCenter alloc] init];
    server = [[NSNotificationCenter alloc] init];
}

- (void) sendNotification:(NSString *)name
{
    NSDictionary *info = [NSDictionary dictionaryWithObjectsAndKeys:nil, nil];
    NSNotification *n = [NSNotification notificationWithName:name object:nil userInfo:info];
    [nc postNotification:n];
}

- (void) sendNotification:(NSString *)name objectsAndKeys:(id)object, ...NS_REQUIRES_NIL_TERMINATION
{
    NSMutableDictionary *info = [NSMutableDictionary dictionary];
    int count = 0;
    va_list args;
    va_start(args, object);
    for (id arg = object; arg != nil; arg = va_arg(args, id))
    {
        if (count % 2 == 0)
            [info setObject:arg forKey:va_arg(args, id)];
    }
    va_end(args);
    
    NSNotification *n = [NSNotification notificationWithName:name object:nil userInfo:info];
    [nc postNotification:n];
}

- (void) sendAnimationNotification:(NSString *)name
{
    NSNotification *n = [NSNotification notificationWithName:name object:nil];
    [animation postNotification:n];
}

- (void) sendAnimationNotification:(NSString *)name objectsAndKeys:(id)object, ...NS_REQUIRES_NIL_TERMINATION
{
    NSMutableDictionary *info = [NSMutableDictionary dictionary];
    int count = 0;
    va_list args;
    va_start(args, object);
    for (id arg = object; arg != nil; arg = va_arg(args, id))
    {
        if (count % 2 == 0)
            [info setObject:arg forKey:va_arg(args, id)];
    }
    va_end(args);
    
    NSNotification *n = [NSNotification notificationWithName:name object:nil userInfo:info];
    [animation postNotification:n];
}


- (void) sendServerNotification: (NSString *)name
{
    NSNotification *n = [NSNotification notificationWithName:name object:nil];
    [server postNotification:n];
}


- (void) sendServerNotification:(NSString *)name objectsAndKeys:(id)object, ...NS_REQUIRES_NIL_TERMINATION
{
    NSMutableDictionary *info = [NSMutableDictionary dictionary];
    int count = 0;
    va_list args;
    va_start(args, object);
    for (id arg = object; arg != nil; arg = va_arg(args, id))
    {
        if (count % 2 == 0)
            [info setObject:arg forKey:va_arg(args, id)];
    }
    va_end(args);
    
    NSNotification *n = [NSNotification notificationWithName:name object:nil userInfo:info];
    [server postNotification:n];
}

- (void) removeObserver:(NSObject *)observer
{
    [nc removeObserver:observer];
    [animation removeObserver:observer];
    [server removeObserver:observer];
}

- (void)dealloc
{
    [nc removeObserver:self];
}

@end
