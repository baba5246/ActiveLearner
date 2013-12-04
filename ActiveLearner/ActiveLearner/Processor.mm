
#import "Processor.h"
#include "ObjectDetector.h"
#include "AdaBoost.h"

@implementation Processor
{
    Model *model;
    Notification *n;
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
    n = [Notification sharedManager];
}

- (void) makeSamples
{
    // 出力
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"--- Start Xml Data Loading! ---", OUTPUT, nil];
    
    // XML data
    XmlMaker *xml = [[XmlMaker alloc] init];
    NSURL *url = [NSURL fileURLWithPath:model.xmlPaths[0]];
    NSString *doc = [[NSString alloc] initWithContentsOfURL:url encoding:NSUTF8StringEncoding error:nil];
    [xml readXmlAndAddData:doc];
    NSDictionary *xmldata = [model getXMLData];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- Start Feature Detection! --- \n", OUTPUT, nil];
    
    for (NSString *path in model.imagePaths)
    {
        long count = samples.size();
        
        // 特徴量抽出
        string filepath = [path cStringUsingEncoding:NSUTF8StringEncoding];
        cout << "Filepath:" << filepath << endl;
        
        vector<Object*> objects;
        ObjectDetector detector(filepath);
        detector.detect(objects);
        
        // filename
        NSArray *comp = [path componentsSeparatedByString:@"/"];
        NSString *nsfilename = comp[comp.count-1];
        string filename = [nsfilename cStringUsingEncoding:NSUTF8StringEncoding];
        
        // サンプル作成
        for (int i = 0; i < objects.size(); i++)
        {
            Object *obj = objects[i];
            Sample s(*obj);
            
            cv::Rect obj_rect = s.object.rect;
            NSRect rect = NSMakeRect(obj_rect.x, obj_rect.y, obj_rect.width, obj_rect.height);
            
            bool findFlag = NO;
            NSArray *truths = xmldata[nsfilename];
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

        NSString *output = [NSString stringWithFormat:@"---- Filename:%s, Samples:%ld", filename.c_str(), samples.size()-count];
        [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    }

}

- (void) learnFeaturesWithAdaBoost
{
    // 出力
    cout<< " " <<endl;
    cout<< "---- Start AdaBoost Learning! ----" <<endl;
    cout<< " " <<endl;
    
    
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
    for (int i = 0; i < labeled.size(); i++) {
        Sample s = labeled[i];
        int test = adaboost.sc.test(s);
        if (test == s.label) correct++;
    }
    double precision = (double)correct / labeled.size();
    cout << "Correct:" << correct << "/" << labeled.size() << ", Precision:" << precision << endl;
}


@end
