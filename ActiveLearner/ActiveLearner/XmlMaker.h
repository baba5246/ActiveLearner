
#import <Foundation/Foundation.h>

@interface XmlMaker : NSObject <NSXMLParserDelegate>

+ (NSString *)makeXmlDocument:(NSDictionary *)data;

- (void) readXmlAndAddData:(NSString *)xml;

@end
