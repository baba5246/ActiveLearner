
#import <Foundation/Foundation.h>

@interface Model : NSObject

@property (readonly) int counter;
@property (nonatomic) NSString *trainDir;
@property (nonatomic) NSString *testDir;
@property (nonatomic) NSArray *trainFiles;
@property (nonatomic) NSArray *testFiles;
@property (nonatomic) NSMutableArray *trainImagePaths;
@property (nonatomic) NSMutableArray *testImagePaths;
@property (nonatomic) NSMutableArray *trainXmlPaths;
@property (nonatomic) NSMutableArray *testXmlPaths;
@property (nonatomic) NSURL *imagePath;
@property (nonatomic) NSString *filename;
@property (nonatomic) NSInteger fileIndex;

+ (Model*)sharedManager;

- (void) setImagePath:(NSURL *)path;
- (void) setFilename:(NSString *)name;

- (NSMutableDictionary *) getXMLData;
- (void) addXMLData:(NSMutableArray *)array key:(NSString *)key;

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)theKey;

@end
