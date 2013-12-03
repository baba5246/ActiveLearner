
#import "FileDialogController.h"
#import "Processor.h"

#define IMAGE_TYPE   @[@"jpg", @"JPG", @"jpeg", @"JPEG", @"png", @"PNG"]
#define XML_TYPE   @[@"xml", @"XML"]

@implementation FileDialogController
{
    Model *model;
    Notification *n;
}

- (IBAction)importDataset:(id)sender
{
    NSOpenPanel *openPanel	= [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:YES];
    NSInteger pressedButton = [openPanel runModal];
    
    model = [Model sharedManager];
    n = [Notification sharedManager];
    
    if( pressedButton == NSOKButton ){
        
        NSURL * dirPath = [openPanel URL];
        
        NSMutableArray *imagePaths = [[NSMutableArray alloc] init];
        NSMutableArray *xmlPaths = [[NSMutableArray alloc] init];
        
        if ([self isDirectory:dirPath]) {
        
            [model setDirectory:dirPath.path];
        
            NSFileManager *manager = [NSFileManager defaultManager];
            NSArray *files = [manager contentsOfDirectoryAtPath:dirPath.path error:nil];
        
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

            [model setFiles:files];
            [model setImagePaths:imagePaths];
            [model setXmlPaths:xmlPaths];
            
            // 通知
            [n sendNotification:DID_LOAD_DIRECTORY];
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
