
#import <Foundation/Foundation.h>

@interface Model : NSObject

@property (readonly) int counter;
@property (nonatomic) NSString *directory;
@property (nonatomic) NSArray *files;
@property (nonatomic) NSMutableArray *imagePaths;
@property (nonatomic) NSMutableArray *xmlPaths;
@property (nonatomic) NSURL *imagePath;
@property (nonatomic) NSString *filename;
@property (nonatomic) NSInteger fileIndex;

+ (Model*)sharedManager;

- (void) setImagePath:(NSURL *)path;
- (void) setFilename:(NSString *)name;

- (void) setPreviousFileInfo;
- (void) setNextFileInfo;

- (void) addTruth:(Truth *)truth;
- (void) resetRectangles;
- (BOOL) saveRectangles;

- (NSMutableArray *) getRectangles;
- (NSMutableDictionary *) getXMLData;
- (void) addXMLData:(NSMutableArray *)array key:(NSString *)key;

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)theKey;

@end
