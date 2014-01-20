#pragma once

#define LOG(A, ...) NSLog(@"LOG: %s:%d:%@", __PRETTY_FUNCTION__,__LINE__,[NSString stringWithFormat:A, ## __VA_ARGS__]);
#define ERROR(A, ...) NSLog(@"ERROR: %s:%d:%@", __PRETTY_FUNCTION__,__LINE__,[NSString stringWithFormat:A, ## __VA_ARGS__]);



#pragma mark -
#pragma mark Model Key

#define IMAGE_PATH_KEY          @"imagePath"
#define RECTANGLES_KEY          @"rectangles"


#pragma mark -
#pragma mark Notification Names

#define DID_LOAD_DIRECTORY      @"did-load-directory"
#define ERROR_LOAD_DIRECTORY    @"error-load-directory"
#define CONSOLE_OUTPUT          @"console-output"
#define OUTPUT                  @"output"


#pragma mark -
#pragma mark Processor Constants

#define TYPE_TRAINING           YES
#define TYPE_TESTING            NO