
#import "Model.h"

@implementation Model
{
    NSMutableDictionary *gtXmlData;
    NSMutableDictionary *adaboostXmlData;
}

@synthesize trainDir, trainFiles, trainImagePaths, trainGtXmlPath;
@synthesize testDir, testFiles, testImagePaths, testGtXmlPath;
@synthesize adaboostXmlPath;

static Model* sharedModel = nil;

+ (Model*)sharedManager {
    @synchronized(self) {
        if (sharedModel == nil) {
            sharedModel = [[self alloc] init];
            [sharedModel prepare];
        }
    }
    return sharedModel;
}

- (void) prepare
{
    gtXmlData = [[NSMutableDictionary alloc] init];
    adaboostXmlData = [[NSMutableDictionary alloc] init];
}

- (NSMutableDictionary *) getGtXMLData
{
    return gtXmlData;
}

- (void) addGtXMLData:(NSMutableArray *)data key:(NSString *)key
{
    [gtXmlData setObject:data forKey:key];
}

- (NSMutableDictionary *) getAdaBoostXMLData
{
    return adaboostXmlData;
}

- (void) setAdaBoostXMLData:(NSMutableDictionary *)data
{
    adaboostXmlData = [[NSMutableDictionary alloc] initWithDictionary:data];
}


@end
