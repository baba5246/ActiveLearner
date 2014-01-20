
#import "Model.h"

@implementation Model
{
    NSMutableArray *rectangles;
    NSMutableDictionary *gtXmlData;
    NSMutableDictionary *adaboostXmlData;
}

@synthesize trainDir, trainFiles, trainImagePaths, trainGtXmlPath;
@synthesize testDir, testFiles, testImagePaths, testGtXmlPath;
@synthesize adaboostXmlPath;
@synthesize counter;
@synthesize imagePath, filename, fileIndex;

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
    rectangles = [[NSMutableArray alloc] init];
    gtXmlData = [[NSMutableDictionary alloc] init];
    adaboostXmlData = [[NSMutableDictionary alloc] init];
}

- (void) setImagePath:(NSURL *)path
{
    [self willChangeValueForKey:IMAGE_PATH_KEY];
    imagePath = path;
    [self didChangeValueForKey:IMAGE_PATH_KEY];
}

- (void) setFilename:(NSString *)name
{
    filename = name;
    if (trainFiles.count > 0) fileIndex = [trainFiles indexOfObject:filename];
    else fileIndex = -1;
}

- (NSString*) filenameFromPath:(NSURL*)path
{
    NSArray *parts = [path.path componentsSeparatedByString:@"/"];
    NSString *fname = [parts objectAtIndex:[parts count]-1];
    
    return fname;
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

/* 手動で観察するための設定
 * これで戻り値が必ずNOになるようにする
 * でもなくても動くので今は置いとく（ →これないと動かなくなった at 130903
 */
+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)theKey
{
    BOOL automatic = NO;

    if ([theKey isEqualToString:IMAGE_PATH_KEY] ||
        [theKey isEqualToString:RECTANGLES_KEY])
    {
        automatic=NO;
    }
    else
    {
        automatic=[super automaticallyNotifiesObserversForKey:theKey];
    }
    return automatic;
}


@end
