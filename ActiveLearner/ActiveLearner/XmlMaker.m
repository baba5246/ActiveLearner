
#import "XmlMaker.h"

@implementation XmlMaker
{
	NSMutableArray *truths;
    BOOL inNameElement;
	BOOL inTextElement;
    NSString *filename, *rect, *text;
    NSMutableDictionary *imageDic;
}

+ (NSString *)makeXmlDocument:(NSDictionary *)data
{
    // XML Document 作成
    NSXMLDocument* document= [NSXMLNode document];
    [document setVersion:@"1.0"];
    
    NSXMLElement* root= [NSXMLNode elementWithName:@"root"];
    [document setRootElement:root];
   
    for (NSString *key in [data allKeys]) {
        
        NSXMLElement* image = [NSXMLNode elementWithName:@"image"]; // imageタグ
        [image addAttribute:[NSXMLNode attributeWithName:@"name" stringValue:key]]; // name属性
        
        for (Truth *t in data[key]) {
            NSXMLElement *truth = [NSXMLNode elementWithName:@"truth"]; // truthタグ
            NSString *rect = [NSString stringWithFormat:@"{{%d,%d},{%d,%d}}",
                              (int)t.rect.origin.x, (int)t.rect.origin.y, (int)t.rect.size.width, (int)t.rect.size.height];
            [truth addAttribute:[NSXMLNode attributeWithName:@"rect" stringValue:rect]];
            [truth addAttribute:[NSXMLNode attributeWithName:@"text" stringValue:t.text]];
            [image addChild:truth];
        }
        
        [root addChild:image];
    }
    
    return [document XMLString];
}

- (void) readXmlAndAddData:(NSString *)xml
{
    NSData *data = [xml dataUsingEncoding:NSUTF8StringEncoding];
    NSXMLParser *parser = [[NSXMLParser alloc] initWithData:data];
    [parser setDelegate:self];
    [parser parse];
}

// 要素の開始タグを読み込み
- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI
                                       qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
	if ([elementName isEqualToString:@"image"]) {
        
        truths = [[NSMutableArray alloc] init];
        filename = attributeDict[@"name"];
        
	} else if ([elementName isEqualToString:@"truth"]) {
        
        rect = attributeDict[@"rect"];
        rect = [rect stringByReplacingOccurrencesOfString:@"{" withString:@""];
        rect = [rect stringByReplacingOccurrencesOfString:@"}" withString:@""];
        NSArray *num = [rect componentsSeparatedByString:@","];
        CGRect rectangle = CGRectMake([num[0] intValue], [num[1] intValue], [num[2] intValue], [num[3] intValue]);
        
        text = attributeDict[@"text"];
        
        Truth *truth = [[Truth alloc] init];
        truth.rect = rectangle;
        truth.text = text;
        [truths addObject:truth];
	}
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
    Model *model = [Model sharedManager];
    
	if ([elementName isEqualToString:@"image"])
    {
        NSMutableDictionary *xmlData = [model getXMLData];
        if (xmlData[filename] == nil) [model addXMLData:truths key:filename];
	}
    else if ([elementName isEqualToString:@"truth"])
    {
		
	}
}


@end
