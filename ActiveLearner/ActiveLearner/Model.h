
#import <Foundation/Foundation.h>

@interface Model : NSObject

@property (readonly) int counter;
@property (nonatomic) NSString *trainDir;
@property (nonatomic) NSString *testDir;
@property (nonatomic) NSArray *trainFiles;
@property (nonatomic) NSArray *testFiles;
@property (nonatomic) NSMutableArray *trainImagePaths;
@property (nonatomic) NSMutableArray *testImagePaths;
@property (nonatomic) NSString *trainGtXmlPath;
@property (nonatomic) NSString *testGtXmlPath;
@property (nonatomic) NSString *adaboostXmlPath;
@property (nonatomic) NSURL *imagePath;
@property (nonatomic) NSString *filename;
@property (nonatomic) NSInteger fileIndex;

+ (Model*)sharedManager;

- (void) setImagePath:(NSURL *)path;
- (void) setFilename:(NSString *)name;

- (NSMutableDictionary *) getGtXMLData;
- (void) addGtXMLData:(NSMutableArray *)data key:(NSString *)key;

- (NSMutableDictionary *) getAdaBoostXMLData;
- (void) setAdaBoostXMLData:(NSMutableDictionary *)data;

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)theKey;

@end
