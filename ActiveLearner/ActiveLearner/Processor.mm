
#import "Processor.h"
#include "Mycv.h"
#include "AdaBoost.h"

@implementation Processor
{
    Model *model;
    vector<Sample> samples;
}

static Processor* sharedProcessor = nil;

+ (Processor*)sharedManager {
    @synchronized(self) {
        if (sharedProcessor == nil) {
            sharedProcessor = [[self alloc] init];
            [sharedProcessor prepare];
        }
    }
    return sharedProcessor;
}

- (void) prepare
{
    model = [Model sharedManager];
}

- (void) makeSamples
{
    // XML data
    XmlMaker *xml = [[XmlMaker alloc] init];
    NSURL *url = [NSURL fileURLWithPath:model.xmlPaths[0]];
    NSString *doc = [[NSString alloc] initWithContentsOfURL:url encoding:NSUTF8StringEncoding error:nil];
    [xml readXmlAndAddData:doc];
    NSDictionary *xmldata = [model getXMLData];
    
    for (NSString *path in model.imagePaths)
    {
        // 特徴量抽出
        string filepath = [path cStringUsingEncoding:NSUTF8StringEncoding];
        Mycv mycv(filepath);
        vector<Object> objects;
        mycv.detector(objects);
        
        // filename
        NSArray *comp = [path componentsSeparatedByString:@"/"];
        string filename = [comp[comp.count-1] cStringUsingEncoding:NSUTF8StringEncoding];
        
        // サンプル作成
        for (int i = 0; i < objects.size(); i++)
        {
            Object obj = objects[i];
            Sample s(obj);
            
            cv::Rect obj_rect = s.object.rect;
            NSRect rect = NSMakeRect(obj_rect.x, obj_rect.y, obj_rect.width, obj_rect.height);
            
            bool findFlag = NO;
            NSArray *truths = xmldata[comp[comp.count-1]];
            for (Truth *t in truths) {
                if (NSContainsRect(t.rect, rect)) { // t.rectにobject.rectが含まれるなら
                    findFlag = YES;
                    break;
                }
            }
            // objのlabel付け
            if (findFlag) s.label = 1;
            else s.label = 0;
            
            samples.push_back(s);
        }
        
        cout<< "---- Filename:" << filename << ", Samples:" << samples.size() << " ----" <<endl;
    }

}

- (void) learnFeaturesWithAdaBoost
{
    // ラベルデータと非ラベルデータ
    vector<Sample>::const_iterator first = samples.begin();
    vector<Sample>::const_iterator half = samples.begin() + (int)(samples.size()*0.5f);
    vector<Sample>::const_iterator end = samples.end();
    vector<Sample> labeled(first, half);
    vector<Sample> unlabeled(half, end);
    
    
    // AdaBoost learning
    vector<WeakClassifier> classifiers;
    for (int i = 0; i < labeled[0].features.size(); i++) {
        classifiers.push_back(WeakClassifier(i));
    }
    
    AdaBoost adaboost(labeled, classifiers);
    
    for (int t = 0; t < labeled[0].features.size(); t++) {
        AdaBoostResult result = adaboost.next();
        WeakClassifier selected = result.wc;
        cout << "t:" << t << ", wc index:" << selected.featureIndex << ", alpha:" << selected.alpha << endl;
    }

    int correct = 0;
    for (int i = 0; i < unlabeled.size(); i++) {
        Sample s = unlabeled[i];
        int test = adaboost.sc.test(s);
        if (test == s.label) correct++;
    }
    double precision = (double)correct / unlabeled.size();
    cout << "Correct:" << correct << "/" << unlabeled.size() << ", Precision:" << precision << endl;

}


@end
