
#import "FileDialogController.h"

@implementation FileDialogController
{
    Model *model;
}

- (IBAction)onSelectImport:(id)sender
{
    NSOpenPanel *openPanel	= [NSOpenPanel openPanel];
    NSArray *allowedFileTypes = [NSArray arrayWithObjects:@"jpeg", @"'JPEG'", @"jpg", @"JPG", @"png", @"PNG", nil];
    [openPanel setAllowedFileTypes:allowedFileTypes];
    NSInteger pressedButton = [openPanel runModal];
    
    model = [Model sharedManager];
    
    if( pressedButton == NSOKButton ){
        
        NSURL * filePath = [openPanel URL];
        NSArray *parts = [filePath.path componentsSeparatedByString:@"/"];
        NSString *filename = [parts objectAtIndex:[parts count]-1];
        NSString *subpath = [filePath.path substringToIndex:filePath.path.length-filename.length];
        [model setDirectory:subpath];
       
        NSFileManager *manager = [NSFileManager defaultManager];
        NSArray *files = [manager contentsOfDirectoryAtPath:subpath error:nil];
        [model setFiles:files];
        
        // nameを先に保存
        [model setFilename:filename];
        [model setImagePath:filePath];
        
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
