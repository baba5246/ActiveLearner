
#import <Foundation/Foundation.h>

@interface Model : NSObject

@property (nonatomic) NSString *trainDir;
@property (nonatomic) NSString *testDir;
@property (nonatomic) NSArray *trainFiles;
@property (nonatomic) NSArray *testFiles;
@property (nonatomic) NSMutableArray *trainImagePaths;
@property (nonatomic) NSMutableArray *testImagePaths;
@property (nonatomic) NSString *trainGtXmlPath;
@property (nonatomic) NSString *testGtXmlPath;
@property (nonatomic) NSString *adaboostXmlPath;

+ (Model*)sharedManager;

- (NSMutableDictionary *) getGtXMLData;
- (void) addGtXMLData:(NSMutableArray *)data key:(NSString *)key;

- (NSMutableDictionary *) getAdaBoostXMLData;
- (void) setAdaBoostXMLData:(NSMutableDictionary *)data;

@end
