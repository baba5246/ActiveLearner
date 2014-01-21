
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
    
    NSXMLElement* root= [NSXMLNode elementWithName:ROOT];
    [document setRootElement:root];
   
    for (NSString *key in [data allKeys]) {
        
        NSXMLElement* image = [NSXMLNode elementWithName:IMAGE]; // imageタグ
        [image addAttribute:[NSXMLNode attributeWithName:NAME stringValue:key]]; // name属性
        
        for (Truth *t in data[key]) {
            NSXMLElement *truth = [NSXMLNode elementWithName:TRUTH]; // truthタグ
            NSString *rect = [NSString stringWithFormat:@"{{%d,%d},{%d,%d}}",
                              (int)t.rect.origin.x, (int)t.rect.origin.y, (int)t.rect.size.width, (int)t.rect.size.height];
            [truth addAttribute:[NSXMLNode attributeWithName:RECT stringValue:rect]];
            [truth addAttribute:[NSXMLNode attributeWithName:TEXT stringValue:t.text]];
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
    
    NSXMLElement* root= [NSXMLNode elementWithName:ROOT];
    [document setRootElement:root];
    
    if ([[data allKeys] count] > 0) {
        
        NSXMLElement* dataset = [NSXMLNode elementWithName:DATASET];
        [dataset addAttribute:[NSXMLNode attributeWithName:PATH stringValue:data[DATASET]]];
        [root addChild:dataset];
        
        NSXMLElement* ccv = [NSXMLNode elementWithName:CCV]; // ccvタグ
        for (NSString *key in [data[CCV] allKeys]) {
            NSXMLElement *wc = [NSXMLNode elementWithName:key];
            [wc addAttribute:[NSXMLNode attributeWithName:FINDEX stringValue:data[CCV][key][FINDEX]]];
            [wc addAttribute:[NSXMLNode attributeWithName:ALPHA stringValue:data[CCV][key][ALPHA]]];
            [wc addAttribute:[NSXMLNode attributeWithName:THRESHOLD stringValue:data[CCV][key][THRESHOLD]]];
            [ccv addChild:wc];
        }
        [root addChild:ccv];
        
        NSXMLElement* cgv = [NSXMLNode elementWithName:CGV]; // ccvタグ
        for (NSString *key in [data[CGV] allKeys]) {
            NSXMLElement *wc = [NSXMLNode elementWithName:key];
            [wc addAttribute:[NSXMLNode attributeWithName:FINDEX stringValue:data[CGV][key][FINDEX]]];
            [wc addAttribute:[NSXMLNode attributeWithName:ALPHA stringValue:data[CGV][key][ALPHA]]];
            [wc addAttribute:[NSXMLNode attributeWithName:THRESHOLD stringValue:data[CGV][key][THRESHOLD]]];
            [cgv addChild:wc];
        }
        [root addChild:cgv];
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
	if ([elementName isEqualToString:IMAGE])
    {
        truths = [[NSMutableArray alloc] init];
        filename = attributeDict[NAME];
	}
    else if ([elementName isEqualToString:TRUTH])
    {
        rect = attributeDict[RECT];
        rect = [rect stringByReplacingOccurrencesOfString:@"{" withString:@""];
        rect = [rect stringByReplacingOccurrencesOfString:@"}" withString:@""];
        NSArray *num = [rect componentsSeparatedByString:@","];
        CGRect rectangle = CGRectMake([num[0] intValue], [num[1] intValue], [num[2] intValue], [num[3] intValue]);
        
        text = attributeDict[TEXT];
        
        Truth *truth = [[Truth alloc] init];
        truth.rect = rectangle;
        truth.text = text;
        [truths addObject:truth];
	}
    else if ([elementName isEqualToString:DATASET])
    {
        adaboost = [[NSMutableDictionary alloc] init];
        dataset = attributeDict[PATH];
        [adaboost setObject:dataset forKey:DATASET];
	}
    else if ([elementName isEqualToString:CCV])
    {
        ccv = [[NSMutableDictionary alloc] init];
        inCCVElement = YES;
    }
    else if ([elementName isEqualToString:CGV])
    {
        cgv = [[NSMutableDictionary alloc] init];
        inCCVElement = NO;
    }
    else if ([elementName hasPrefix:WC])
    {
        wc = [[NSMutableDictionary alloc] init];
        findex = attributeDict[FINDEX];
        alpha = attributeDict[ALPHA];
        threshold = attributeDict[THRESHOLD];
        [wc setObject:findex forKey:FINDEX];
        [wc setObject:alpha forKey:ALPHA];
        [wc setObject:threshold forKey:THRESHOLD];
        
        if (inCCVElement) [ccv setObject:wc forKey:elementName];
        else [cgv setObject:wc forKey:elementName];
    }
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
    Model *model = [Model sharedManager];
    
	if ([elementName isEqualToString:IMAGE])
    {
        NSMutableDictionary *xmlData = [model getGtXMLData];
        if (xmlData[filename] == nil) [model addGtXMLData:truths key:filename];
	}
    else if ([elementName isEqualToString:TRUTH])
    {
		
	}
    else if ([elementName isEqualToString:CCV])
    {
        [adaboost setObject:ccv forKey:CCV];
    }
    else if ([elementName isEqualToString:CGV])
    {
        [adaboost setObject:cgv forKey:CGV];
        [model setAdaBoostXMLData:adaboost];
    }
}


@end
