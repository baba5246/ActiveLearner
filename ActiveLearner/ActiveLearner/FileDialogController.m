
#import "FileDialogController.h"
#import "Processor.h"

@implementation FileDialogController
{
    Model *model;
}

- (IBAction)importDataset:(id)sender
{
    NSOpenPanel *openPanel	= [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:YES];
    NSInteger pressedButton = [openPanel runModal];
    
    model = [Model sharedManager];
    
    if( pressedButton == NSOKButton ){
        
        NSURL * dirPath = [openPanel URL];
        [model setDirectory:dirPath.path];
        
        NSFileManager *manager = [NSFileManager defaultManager];
        NSArray *files = [manager contentsOfDirectoryAtPath:dirPath.path error:nil];
        NSMutableArray *imagePaths = [[NSMutableArray alloc] init];
        NSMutableArray *xmlPaths = [[NSMutableArray alloc] init];
        
        NSString *fullPath;
        for (NSString *path in files) {
            NSArray *components = [path componentsSeparatedByString:@"."];
            if ([components[components.count-1] isEqualToString:@"jpg"] ||
                [components[components.count-1] isEqualToString:@"JPG"]) {
                fullPath = [dirPath.path stringByAppendingFormat:@"/%@", path];
                [imagePaths addObject:fullPath];
            } else if ([components[components.count-1] isEqualToString:@"xml"]) {
                fullPath = [dirPath.path stringByAppendingFormat:@"/%@", path];
                [xmlPaths addObject:fullPath];
            }
        }

        [model setFiles:files];
        [model setImagePaths:imagePaths];
        [model setXmlPaths:xmlPaths];
        
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



@end
