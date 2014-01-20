
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
        NSString *gtXMLPath, *adaboostXMLPath;
        
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
                    NSRange gt = [path rangeOfString:@"gt"];
                    if (gt.location != NSNotFound) {
                        gtXMLPath = fullPath;
                    } else {
                        adaboostXMLPath = fullPath;
                    }
                }
            }
            
            [model setTrainDir:dirPath.path];
            [model setTrainFiles:files];
            [model setTrainImagePaths:imagePaths];
            [model setTrainGtXmlPath:gtXMLPath];
            [model setAdaboostXmlPath:adaboostXMLPath];
            
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
                    NSRange gt = [path rangeOfString:@"gt"];
                    if (gt.location != NSNotFound) {
                        gtXMLPath = fullPath;
                    } else {
                        adaboostXMLPath = fullPath;
                    }
                }
            }
            
            [model setTrainDir:dirPath.path];
            [model setTrainImagePaths:imagePaths];
            [model setTrainGtXmlPath:gtXMLPath];
            [model setAdaboostXmlPath:adaboostXMLPath];
            
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
    model = [Model sharedManager];
    n = [Notification sharedManager];
    if (model.adaboostXmlPath.length==0) {
        NSAlert *alert = [NSAlert alertWithMessageText:@"学習データがありません!"
                                         defaultButton:@"OK" alternateButton:nil otherButton:nil
                             informativeTextWithFormat:@"Trainingフォルダを指定してください."];
        NSInteger answer = [alert runModal];
        if (answer == NSAlertDefaultReturn) {
            return;
        }
    }
    
    NSOpenPanel *openPanel	= [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:YES];
    NSInteger pressedButton = [openPanel runModal];
    
    
    if( pressedButton == NSOKButton ){
        
        NSURL * dirPath = [openPanel URL];
        
        NSArray *files;
        NSMutableArray *imagePaths = [[NSMutableArray alloc] init];
        NSString *gtXMLPath;
        
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
                    NSRange gt = [path rangeOfString:@"gt"];
                    if (gt.location != NSNotFound) {
                        gtXMLPath = fullPath;
                    }
                }
            }
            
            [model setTestDir:dirPath.path];
            [model setTestFiles:files];
            [model setTestImagePaths:imagePaths];
            [model setTestGtXmlPath:gtXMLPath];
            
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
                    NSRange gt = [path rangeOfString:@"gt"];
                    if (gt.location != NSNotFound) {
                        gtXMLPath = fullPath;
                    }
                }
            }
            
            [model setTestDir:dirPath.path];
            [model setTestImagePaths:imagePaths];
            [model setTestGtXmlPath:gtXMLPath];
            
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

- (IBAction)onExportLearningResultButtonClicked:(id)sender
{
    // データ保存
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    NSArray *allowedFileTypes = [NSArray arrayWithObjects:@"xml", nil];
    [savePanel setAllowedFileTypes:allowedFileTypes];
    [savePanel setNameFieldStringValue:@"adaboost.xml"];
    if ([savePanel runModal] == NSOKButton) {
        // XML作成
        NSURL * filePath = [savePanel URL];
        NSString *document = [XmlMaker makeAdaBoostXmlDocument:[model getAdaBoostXMLData]];
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
