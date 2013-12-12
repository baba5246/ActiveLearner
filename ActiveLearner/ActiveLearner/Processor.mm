
#import "Processor.h"
#include "ObjectDetector.h"
#include "AdaBoost.h"

@implementation Processor
{
    Model *model;
    Notification *n;
    
    NSDictionary *xmldata;
    
    BOOL isTraining;
    AdaBoost *ccvAdaBoost, *cgvAdaBoost;
}


#pragma mark -
#pragma mark Initialize Methods

static Processor* sharedProcessor = nil;

+ (Processor*)sharedManager
{
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


#pragma mark -
#pragma mark Excute Methods

- (void) excuteWhole:(BOOL)type
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n ********** 全プロセス実行 開始！ ********** \n", OUTPUT, nil];
    
    isTraining = type;
    
    [self loadXMLData];
    
    vector<Object*> ccs = [self excuteCCD];
//    vector<Object*> components = [self excuteCCV:ccs];
    
    vector<Text*> cgs = [self excuteCGD:ccs];
    
//    vector<Text*> texts = [self excuteCGV:cgs];
    
    // TODO: 抽出したTextsを評価
    // TODO: 抽出したTextsの表示
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n ********** 全プロセス実行 終了！ ********** \n", OUTPUT, nil];
}

- (vector<Object*>) excuteCCD
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- オブジェクト抽出開始 --- \n", OUTPUT, nil];
    
    vector<Object*> ccs;
    
    for (NSString *path in model.imagePaths)
    {
        long count = ccs.size();
        
        // 特徴量抽出
        string filepath = [path cStringUsingEncoding:NSUTF8StringEncoding];
        
        vector<Object*> objects;
        ObjectDetector detector(filepath);
        detector.detect(objects);
        
        ccs.insert(ccs.end(), objects.begin(), objects.end());

        NSString *output = [NSString stringWithFormat:@"---- Filename:%@, Samples:%ld", path, ccs.size()-count];
        [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    }
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return ccs;
}

- (vector<Object*>) excuteCCV:(vector<Object*>) ccs
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- オブジェクト分類開始 --- \n", OUTPUT, nil];
    
    vector<Object *> components;
    

    vector<Sample> samples = [self makeCCSamples:ccs];
    if (isTraining) {
        AdaBoost adaboost = [self learnFeaturesWithAdaBoost:samples];
        ccvAdaBoost = &adaboost;
    }
    components = [self ccClassify:samples adaboost:ccvAdaBoost];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return  components;
}

- (vector<Text*>) excuteCGD:(vector<Object*>) components
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- オブジェクト抽出開始 --- \n", OUTPUT, nil];
    
    vector<Text*> cgs;
    
    // Grouping
    
    
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return cgs;
}

- (vector<Text*>) excuteCGV:(vector<Text*>) cgs
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- グループ分類開始 --- \n", OUTPUT, nil];
    
    vector<Text *> texts;
    
    if (isTraining)
    {
        vector<Sample> samples = [self makeCGSamples:cgs];
        AdaBoost adaboost = [self learnFeaturesWithAdaBoost:samples];
        cgvAdaBoost = &adaboost;
        texts = [self cgClassify:cgs adaboost:adaboost];
    }
    else
    {
        texts = [self cgClassify:cgs adaboost:*(cgvAdaBoost)];
    }
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return texts;
}


#pragma mark -
#pragma mark Assistant Methods

- (void) loadXMLData
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"--- XMLファイル読み込み開始 ---", OUTPUT, nil];
    
    XmlMaker *xml = [[XmlMaker alloc] init];
    NSURL *url = [NSURL fileURLWithPath:model.xmlPaths[0]];
    NSString *doc = [[NSString alloc] initWithContentsOfURL:url encoding:NSUTF8StringEncoding error:nil];
    [xml readXmlAndAddData:doc];
    xmldata = [model getXMLData];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
}

- (vector<Sample>) makeCCSamples:(vector<Object*>) objects
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- ラベリング開始 --- \n", OUTPUT, nil];
    
    vector<Sample> samples;
    
    for (NSString *path in model.imagePaths)
    {
        // filename 抽出
        NSArray *comp = [path componentsSeparatedByString:@"/"];
        NSString *nsfilename = comp[comp.count-1];
        string filename = [nsfilename cStringUsingEncoding:NSUTF8StringEncoding];
        
        // サンプル作成
        for (int i = 0; i < objects.size(); i++)
        {
            if (objects[i]->filename.compare(filename) != 0)
                continue;
            
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
    }
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return samples;
}

- (vector<Sample>) makeCGSamples:(vector<Text*>) groups
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- ラベリング開始 --- \n", OUTPUT, nil];
    
    vector<Sample> samples;
    
    for (NSString *path in model.imagePaths)
    {
        // filename 抽出
        NSArray *comp = [path componentsSeparatedByString:@"/"];
        NSString *nsfilename = comp[comp.count-1];
        string filename = [nsfilename cStringUsingEncoding:NSUTF8StringEncoding];
        
        // サンプル作成
        for (int i = 0; i < groups.size(); i++)
        {
            Text *text = groups[i];
            Sample s(*text);
            
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
    }
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return samples;
}

- (AdaBoost) learnFeaturesWithAdaBoost:(vector<Sample>) samples
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- AdaBoost学習開始 --- \n", OUTPUT, nil];
    
    // ラベルデータと非ラベルデータ
    vector<Sample>::const_iterator first = samples.begin();
    vector<Sample>::const_iterator half = samples.begin() + (int)(samples.size()*0.5f);
    vector<Sample>::const_iterator end = samples.end();
    vector<Sample> labeled(first, half);
    vector<Sample> unlabeled(half, end);
    
    // AdaBoost learning
    vector<WeakClassifier> classifiers;
    for (int i = 0; i < labeled[0].features.size(); i++) {
        WeakClassifier wc(i);
        classifiers.push_back(wc);
    }
    
    AdaBoost adaboost(labeled, classifiers);
    
    for (int t = 0; t < labeled[0].features.size(); t++) {
        AdaBoostResult result = adaboost.next();
        WeakClassifier selected = result.wc;
        cout << "t:" << t << ", wc index:" << selected.featureIndex << ", alpha:" << selected.alpha << endl;
    }

    return adaboost;
}

- (vector<Object*>) ccClassify:(vector<Sample>) samples adaboost:(AdaBoost*) adaboost
{
    vector<Object*> components;
    
    int e = 0, E = 0, T = 0;
    double precision = 0, recall = 0, f = 0;
    
    for (int i = 0; i < samples.size(); i++)
    {
        cout << "Samples:" << samples.size() << ", i:" << i << endl;
        Sample s = samples[i];
        int test = adaboost->sc.test(s);
        
        if (test == 1) {
            components.push_back(&s.object);
        }
        
        // 正解個数を算出
        if (test == s.label) e++;
    }
    
    // 母数を計算
    E = (int)samples.size();
    for (NSString *filename in [xmldata allKeys]) {
        NSArray *truths = xmldata[filename];
        T += truths.count;
    }
    
    // PとRとFを算出
    precision = (double)e / E;
    recall = (double)e / T;
    f = 2 * precision * recall / (precision + recall);
    
    NSString *result = [NSString stringWithFormat:@"\n --- 候補オブジェクト分類結果 --- \n Precision:%f, Recall:%f, F:%f",
                        precision, recall, f];
//    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:result, OUTPUT, nil];
    
    cout << "--- 候補オブジェクト分類結果 ---" << endl;
    cout << "size:" << samples.size() << "e:" << e << ", E:" << E << ", T:" << T << endl;
    cout << result << endl;
    
    return components;
}

- (vector<Text*>) cgClassify:(vector<Text*>) cgs adaboost:(AdaBoost) adaboost
{
    vector<Text*> texts;
    
    for (int i = 0; i < cgs.size(); i++)
    {
        Text *t = cgs[i];
        int test = adaboost.sc.test(t);
        
        if (test == 1) {
            texts.push_back(t);
        }
        
        // TODO: ここでの精度はかる
    }
    
    return texts;
}



#pragma mark -
#pragma mark Evaluate Methods

- (void) evaluateComponents:(vector<Object*>) components
{
    
}

- (void) evaluateTexts:(vector<Text*>)texts
{
    
}


@end
