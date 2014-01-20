
#import "XmlMaker.h"

@implementation XmlMaker
{
	NSMutableArray *truths;
    BOOL inNameElement;
	BOOL inTextElement;
    BOOL inCCVElement;
    NSString *filename, *rect, *text;
    NSString *dataset, *findex, *alpha, *threshold;
    NSMutableDictionary *imageDic;
    NSMutableDictionary *adaboost, *ccv, *cgv, *wc;
}

+ (NSString *)makeGtXmlDocument:(NSDictionary *)data
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

+ (NSString *)makeAdaBoostXmlDocument:(NSDictionary *)data
{
    // XML Document 作成
    NSXMLDocument* document= [NSXMLNode document];
    [document setVersion:@"1.0"];
    
    NSXMLElement* root= [NSXMLNode elementWithName:@"root"];
    [document setRootElement:root];
    
    if ([[data allKeys] count] > 0) {
        
        NSXMLElement* dataset = [NSXMLNode elementWithName:@"dataset"];
        [dataset addAttribute:[NSXMLNode attributeWithName:@"path" stringValue:data[@"dataset"]]];
        [root addChild:dataset];
        
        NSXMLElement* wcs = [NSXMLNode elementWithName:@"wcs"]; // wcsタグ
        for (NSString *key in [data[@"wcs"] allKeys]) {
            NSXMLElement *wc = [NSXMLNode elementWithName:key];
            [wc addAttribute:[NSXMLNode attributeWithName:@"findex" stringValue:data[key][@"findex"]]];
            [wc addAttribute:[NSXMLNode attributeWithName:@"alpha" stringValue:data[key][@"alpha"]]];
            [wc addAttribute:[NSXMLNode attributeWithName:@"threshold" stringValue:data[key][@"threshold"]]];
            [wcs addChild:wc];
        }
        [root addChild:wcs];
    }
    
    return [document XMLString];
}


- (void) readGtXmlAndAddData:(NSString *)xml
{
    NSData *data = [xml dataUsingEncoding:NSUTF8StringEncoding];
    NSXMLParser *parser = [[NSXMLParser alloc] initWithData:data];
    [parser setDelegate:self];
    [parser parse];
}
- (void) readAdaBoostXml:(NSString *)xml
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
	if ([elementName isEqualToString:@"image"])
    {
        truths = [[NSMutableArray alloc] init];
        filename = attributeDict[@"name"];
	}
    else if ([elementName isEqualToString:@"truth"])
    {
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
    else if ([elementName isEqualToString:@"dataset"])
    {
        adaboost = [[NSMutableDictionary alloc] init];
        dataset = attributeDict[@"path"];
        [adaboost setObject:dataset forKey:@"dataset"];
	}
    else if ([elementName isEqualToString:@"ccv"])
    {
        ccv = [[NSMutableDictionary alloc] init];
        inCCVElement = YES;
    }
    else if ([elementName isEqualToString:@"cgv"])
    {
        cgv = [[NSMutableDictionary alloc] init];
        inCCVElement = NO;
    }
    else if ([elementName hasPrefix:@"wc"])
    {
        wc = [[NSMutableDictionary alloc] init];
        findex = attributeDict[@"findex"];
        alpha = attributeDict[@"alpha"];
        threshold = attributeDict[@"threshold"];
        [wc setObject:findex forKey:@"findex"];
        [wc setObject:alpha forKey:@"alpha"];
        [wc setObject:threshold forKey:@"threshold"];
        
        if (inCCVElement) [ccv setObject:wc forKey:elementName];
        else [cgv setObject:wc forKey:elementName];
    }
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
    Model *model = [Model sharedManager];
    
	if ([elementName isEqualToString:@"image"])
    {
        NSMutableDictionary *xmlData = [model getGtXMLData];
        if (xmlData[filename] == nil) [model addGtXMLData:truths key:filename];
	}
    else if ([elementName isEqualToString:@"truth"])
    {
		
	}
    else if ([elementName isEqualToString:@"ccv"])
    {
        [adaboost setObject:ccv forKey:@"ccv"];
    }
    else if ([elementName isEqualToString:@"cgv"])
    {
        [adaboost setObject:cgv forKey:@"cgv"];
        [model setAdaBoostXMLData:adaboost];
    }
}


@end
