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
#define IMAGE_OUTPUT            @"image-output"
#define IMAGE_DATA              @"image-data"
#define UPDATE_IMAGE_NAME       @"update-image-name"
#define FILEPATH                @"filepath"

#pragma mark -
#pragma mark Processor Constants

#define TYPE_TRAINING           YES
#define TYPE_TESTING            NO

#pragma mark -
#pragma mark XML Keys

#define ROOT        @"root"

#define IMAGE       @"image"
#define NAME        @"name"
#define TRUTH       @"truth"
#define RECT        @"rect"
#define TEXT        @"text"

#define DATASET     @"dataset"
#define PATH        @"path"
#define CCV         @"ccv"
#define CGV         @"cgv"
#define WC          @"wc"
#define FINDEX      @"findex"
#define ALPHA       @"alpha"
#define THRESHOLD   @"threshold"


#pragma mark -
#pragma mark Draw Wait Time

#define DRAW_WAIT_TIME  3000