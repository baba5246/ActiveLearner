
#import "Model.h"

@implementation Model
{
    NSMutableArray *rectangles;
    NSMutableDictionary *xmlData;
}

@synthesize directory;
@synthesize files;
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
    xmlData = [[NSMutableDictionary alloc] init];
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
    if (files.count > 0) fileIndex = [files indexOfObject:filename];
    else fileIndex = -1;
}

- (NSString*) filenameFromPath:(NSURL*)path
{
    NSArray *parts = [path.path componentsSeparatedByString:@"/"];
    NSString *fname = [parts objectAtIndex:[parts count]-1];
    
    return fname;
}


- (void) setPreviousFileInfo
{
    fileIndex--;
    filename = [files objectAtIndex:fileIndex];
    NSString *pathString = [directory stringByAppendingString:filename];
    imagePath = [NSURL fileURLWithPath:pathString];
}

- (void)setNextFileInfo
{
    fileIndex++;
    filename = [files objectAtIndex:fileIndex];
    NSString *pathString = [directory stringByAppendingString:filename];
    imagePath = [NSURL fileURLWithPath:pathString];
}


- (void) addTruth:(Truth *)truth
{
    [rectangles addObject:truth];
    [self willChangeValueForKey:RECTANGLES_KEY];
    [self didChangeValueForKey:RECTANGLES_KEY];
}

- (void) resetRectangles
{
    [self willChangeValueForKey:RECTANGLES_KEY];
    [rectangles removeAllObjects];
    [self didChangeValueForKey:RECTANGLES_KEY];
}

- (BOOL) saveRectangles
{
    if (rectangles.count > 0)
    {
        NSArray *rects = [[NSArray alloc] initWithArray:rectangles];
        [xmlData setObject:rects forKey:filename];
        return YES;
    }
    else
    {
        return NO;
    }
}

- (NSMutableArray *) getRectangles
{
    return rectangles;
}

- (NSMutableDictionary *) getXMLData
{
    return xmlData;
}

- (void) addXMLData:(NSMutableArray *)data key:(NSString *)key
{
    [xmlData setObject:data forKey:key];
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
