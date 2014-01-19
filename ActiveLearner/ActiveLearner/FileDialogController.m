
#import "FileDialogController.h"
#import "Processor.h"

#define IMAGE_TYPE   @[@"jpg", @"JPG", @"jpeg", @"JPEG", @"png", @"PNG"]
#define XML_TYPE   @[@"xml", @"XML"]

@implementation FileDialogController
{
    Model *model;
    Notification *n;
}

- (IBAction)importTrainDataset:(id)sender
{
    NSOpenPanel *openPanel	= [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:YES];
    NSInteger pressedButton = [openPanel runModal];
    
    model = [Model sharedManager];
    n = [Notification sharedManager];
    
    if( pressedButton == NSOKButton ){
        
        NSURL * dirPath = [openPanel URL];
        
        NSArray *files;
        NSMutableArray *imagePaths = [[NSMutableArray alloc] init];
        NSMutableArray *xmlPaths = [[NSMutableArray alloc] init];
        
        if ([self isDirectory:dirPath])
        {
            NSFileManager *manager = [NSFileManager defaultManager];
            files = [manager contentsOfDirectoryAtPath:dirPath.path error:nil];
        
            NSString *fullPath;
            for (NSString *path in files)
            {
                if ([self isImage:path]) // 画像判定
                {
                    fullPath = [dirPath.path stringByAppendingFormat:@"/%@", path];
                    [imagePaths addObject:fullPath];
                }
                else if ([self isXMLDocument:path]) // xml判定
                {
                    fullPath = [dirPath.path stringByAppendingFormat:@"/%@", path];
                    [xmlPaths addObject:fullPath];
                }
            }
            
            [model setTrainDir:dirPath.path];
            [model setTrainFiles:files];
            [model setTrainImagePaths:imagePaths];
            [model setTrainXmlPaths:xmlPaths];
            
            // 通知
            NSNumber *type = [NSNumber numberWithBool:YES];
            [n sendNotification:DID_LOAD_DIRECTORY objectsAndKeys:type, @"type", nil];
        }
        else if ([self isImage:dirPath.path])
        {
            [imagePaths addObject:dirPath.path];
            
            NSFileManager *manager = [NSFileManager defaultManager];
            NSString *directory = dirPath.path.stringByDeletingLastPathComponent;
            files = [manager contentsOfDirectoryAtPath:directory error:nil];
            NSString *fullPath;
            for (NSString *path in files) {
                if ([self isXMLDocument:path]) {
                    fullPath = [directory stringByAppendingFormat:@"/%@", path];
                    [xmlPaths addObject:fullPath];
                }
            }
            
            [model setTrainDir:dirPath.path];
            [model setTrainImagePaths:imagePaths];
            [model setTrainXmlPaths:xmlPaths];
            
            // 通知
            NSNumber *type = [NSNumber numberWithBool:YES];
            [n sendNotification:DID_LOAD_DIRECTORY objectsAndKeys:type, @"type", nil];
        }
        else
        {
            // エラー表示
            [n sendNotification:ERROR_LOAD_DIRECTORY];
        }
        
    }else if( pressedButton == NSCancelButton ){
     	NSLog(@"Cancel button was pressed.");
    }else{
     	// error
    }
}

- (IBAction)importTestDataset:(id)sender
{
    NSOpenPanel *openPanel	= [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:YES];
    NSInteger pressedButton = [openPanel runModal];
    
    model = [Model sharedManager];
    n = [Notification sharedManager];
    
    if( pressedButton == NSOKButton ){
        
        NSURL * dirPath = [openPanel URL];
        
        NSArray *files;
        NSMutableArray *imagePaths = [[NSMutableArray alloc] init];
        NSMutableArray *xmlPaths = [[NSMutableArray alloc] init];
        
        if ([self isDirectory:dirPath])
        {
            NSFileManager *manager = [NSFileManager defaultManager];
            files = [manager contentsOfDirectoryAtPath:dirPath.path error:nil];
            
            NSString *fullPath;
            for (NSString *path in files)
            {
                if ([self isImage:path]) // 画像判定
                {
                    fullPath = [dirPath.path stringByAppendingFormat:@"/%@", path];
                    [imagePaths addObject:fullPath];
                }
                else if ([self isXMLDocument:path]) // xml判定
                {
                    fullPath = [dirPath.path stringByAppendingFormat:@"/%@", path];
                    [xmlPaths addObject:fullPath];
                }
            }
            
            [model setTestDir:dirPath.path];
            [model setTestFiles:files];
            [model setTestImagePaths:imagePaths];
            [model setTestXmlPaths:xmlPaths];
            
            // 通知
            NSNumber *type = [NSNumber numberWithBool:NO];
            [n sendNotification:DID_LOAD_DIRECTORY objectsAndKeys:type,@"type", nil];
        }
        else if ([self isImage:dirPath.path])
        {
            [imagePaths addObject:dirPath.path];
            
            NSFileManager *manager = [NSFileManager defaultManager];
            NSString *directory = dirPath.path.stringByDeletingLastPathComponent;
            files = [manager contentsOfDirectoryAtPath:directory error:nil];
            NSString *fullPath;
            for (NSString *path in files) {
                if ([self isXMLDocument:path]) {
                    fullPath = [directory stringByAppendingFormat:@"/%@", path];
                    [xmlPaths addObject:fullPath];
                }
            }
            
            [model setTestDir:dirPath.path];
            [model setTestImagePaths:imagePaths];
            [model setTestXmlPaths:xmlPaths];
            
            // 通知
            NSNumber *type = [NSNumber numberWithBool:NO];
            [n sendNotification:DID_LOAD_DIRECTORY objectsAndKeys:type, @"type", nil];
        }
        else
        {
            // エラー表示
            [n sendNotification:ERROR_LOAD_DIRECTORY];
        }
        
    }else if( pressedButton == NSCancelButton ){
     	NSLog(@"Cancel button was pressed.");
    }else{
     	// error
    }
}

-(IBAction)onExportXMLButtonClicked:(id)sender
{
    // データ保存
    NSString *oldDoc;
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    NSArray *allowedFileTypes = [NSArray arrayWithObjects:@"xml", nil];
    [savePanel setAllowedFileTypes:allowedFileTypes];
    if ([savePanel runModal] == NSOKButton) {
        // Get Nsstring
        NSURL * filePath = [savePanel URL];
        oldDoc = [[NSString alloc] initWithContentsOfURL:filePath encoding:NSUTF8StringEncoding error:nil];
        
        model = [Model sharedManager];
        XmlMaker *xmlMaker = [[XmlMaker alloc] init];
        [xmlMaker readXmlAndAddData:oldDoc];
        
        // XML作成
        NSString *document = [XmlMaker makeXmlDocument:[model getXMLData]];
        [document writeToFile:filePath.path atomically:YES encoding:4 error:NULL];
    }
    
}


- (BOOL) isDirectory:(NSURL *) url
{
    NSArray *components = [url.path componentsSeparatedByString:@"."];
    
    if (components.count < 2) return true;
    else return false;
}

- (BOOL) isImage:(NSString *) path
{
    NSArray *types = IMAGE_TYPE;
    NSArray *components = [path componentsSeparatedByString:@"."];
    NSString *last = components[components.count-1];
    
    for (NSString *type in types) {
        if ([last isEqualToString:type]) {
            return true;
        }
    }
    
    return false;
}

- (BOOL) isXMLDocument:(NSString *) path
{
    NSArray *types = XML_TYPE;
    NSArray *components = [path componentsSeparatedByString:@"."];
    NSString *last = components[components.count-1];
    
    for (NSString *type in types) {
        if ([last isEqualToString:type]) {
            return true;
        }
    }
    
    return false;
}

@end
