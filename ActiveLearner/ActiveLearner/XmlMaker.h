
#import <Foundation/Foundation.h>

@interface XmlMaker : NSObject <NSXMLParserDelegate>

+ (NSString *)makeGtXmlDocument:(NSDictionary *)data;
+ (NSString *)makeAdaBoostXmlDocument:(NSDictionary *)data;

- (void) readGtXmlAndAddData:(NSString *)xml;
- (void) readAdaBoostXml:(NSString *)xml;

@end
