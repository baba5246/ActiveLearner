
#import "Processor.h"
#include "ObjectDetector.h"
#include "AdaBoost.h"

#define CCV @"ccv"
#define CGV @"cgv"

@implementation Processor
{
    Model *model;
    Notification *n;
    
    NSDictionary *xmldata;
    NSDictionary *adaboostXmldata;
    
    AdaBoost ccvAdaBoost;
    AdaBoost cgvAdaBoost;
    
    NSMutableDictionary *adaboostResult;
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
#pragma mark Training Methods

- (void) trainWhole
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n ********** Training 全プロセス実行 開始！ ********** \n", OUTPUT, nil];
    
    BOOL isTraining = YES;
    [self loadGtXMLData:isTraining];
    
    map<string, vector<Object*>> ccs = [self trainCCD];
    map<string, vector<Object*>> components = [self trainCCV:ccs];
    map<string, vector<Text*>> cgs = [self trainCGD:components];
    map<string, vector<Text*>> texts = [self trainCGV:cgs];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n ********** Training 全プロセス実行 終了！ ********** \n", OUTPUT, nil];
}

- (map<string, vector<Object*>>) trainCCD
{
    NSString *output = @"\n --- オブジェクト抽出開始 --- \n";
    LOG(@"%@", output);
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    
    map<string, vector<Object*>> ccs;
    
    for (NSString *path in model.trainImagePaths)
    {
        // 特徴量抽出
        string filepath = [path cStringUsingEncoding:NSUTF8StringEncoding];
        
        vector<Object*> objects;
        ObjectDetector detector(filepath);
        detector.detect(objects);
        
        ccs.insert(map<string, vector<Object*>>::value_type(filepath, objects));

        output = [NSString stringWithFormat:@"\n---- Filename:%@, Components:%ld", path, objects.size()];
        LOG(@"%@", output);
        [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    }
    
    return ccs;
}

- (map<string, vector<Object*>>) trainCCV:(map<string, vector<Object*>>) ccs
{
    NSString *output = @"\n --- オブジェクト学習開始 --- \n";
    LOG(@"%@", output);
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    
    map<string, vector<Object *>> components;

    map<string, vector<Sample>> samples = [self makeCCSamples:ccs isTraining:YES];
    AdaBoost adaboost = [self learnFeaturesWithAdaBoost:samples];
    ccvAdaBoost = adaboost;
    [self saveAdaBoostResultToXml:CCV];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    
    components = [self ccClassify:samples adaboost:ccvAdaBoost];
    return  components;
}

- (map<string, vector<Text*>>) trainCGD:(map<string, vector<Object*>>) components
{
    NSString *output = @"\n --- グループ抽出開始 --- \n";
    LOG(@"%@", output);
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    
    map<string, vector<Text*>> cgs;
    
    for (NSString *path in model.trainImagePaths) {
        
        // グループ抽出
        string filepath = [path cStringUsingEncoding:NSUTF8StringEncoding];
        Mat srcImage = imread(filepath);
        vector<Object*> objects = components.at(filepath);
        
        vector<Text*> texts;
        TextDetector detector(srcImage);
        detector.detect(objects, texts);
        
        cgs.insert(map<string, vector<Text*>>::value_type(filepath, texts));
        
        output = [NSString stringWithFormat:@"\n---- Filename:%@, Texts:%ld", path, texts.size()];
        LOG(@"%@", output);
        [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
        
    }
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return cgs;
}

- (map<string, vector<Text*>>) trainCGV:(map<string, vector<Text*>>) cgs
{
    NSString *output = @"\n --- グループ学習開始 --- \n";
    LOG(@"%@", output);
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    
    map<string, vector<Text*>> texts;
    
    map<string, vector<Sample>> samples = [self makeCGSamples:cgs isTraining:YES];
    AdaBoost adaboost = [self learnFeaturesWithAdaBoost:samples];
    cgvAdaBoost = adaboost;
    [self saveAdaBoostResultToXml:CGV];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    
    texts = [self cgClassify:samples adaboost:cgvAdaBoost];
    return texts;
}

#pragma mark -
#pragma mark Testing Methods

- (void) testWhole
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n ********** Test 全プロセス実行 開始！ ********** \n", OUTPUT, nil];
    
    BOOL isTraining = NO;
    [self loadGtXMLData:isTraining];
    
    map<string, vector<Object*>> ccs = [self testCCD];
    map<string, vector<Object*>> components = [self testCCV:ccs];
    map<string, vector<Text*>> cgs = [self testCGD:components];
    map<string, vector<Text*>> texts = [self testCGV:cgs];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n ********** Test 全プロセス実行 終了！ ********** \n", OUTPUT, nil];
}

- (map<string, vector<Object*>>) testCCD
{
    [n sendNotification:CONSOLE_OUTPUT
         objectsAndKeys:@"\n --- オブジェクト抽出開始 --- \n", OUTPUT, nil];
    
    map<string, vector<Object*>> ccs;
    
    for (NSString *path in model.testImagePaths)
    {
        // 特徴量抽出
        string filepath = [path cStringUsingEncoding:NSUTF8StringEncoding];
        
        vector<Object*> objects;
        ObjectDetector detector(filepath);
        detector.detect(objects);
        
        ccs.insert(map<string, vector<Object*>>::value_type(filepath, objects));
        
        NSString *output = [NSString stringWithFormat:@"---- Filename:%@, Samples:%ld", path, objects.size()];
        [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    }
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return ccs;
}

- (map<string, vector<Object*>>) testCCV:(map<string, vector<Object*>>) ccs
{
    [n sendNotification:CONSOLE_OUTPUT
         objectsAndKeys:@"\n --- オブジェクト分類開始 --- \n", OUTPUT, nil];
    
    map<string, vector<Object*>> components;
    
    map<string, vector<Sample>> samples = [self makeCCSamples:ccs isTraining:YES];
    components = [self ccClassify:samples adaboost:ccvAdaBoost];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return  components;
}

- (map<string, vector<Text*>>) testCGD:(map<string, vector<Object*>>) components
{
    [n sendNotification:CONSOLE_OUTPUT
         objectsAndKeys:@"\n --- オブジェクト抽出開始 --- \n", OUTPUT, nil];
    
    map<string, vector<Text*>> cgs;
    
    // Grouping
    
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return cgs;
}

- (map<string, vector<Text*>>) testCGV:(map<string, vector<Text*>>) cgs
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- グループ分類開始 --- \n", OUTPUT, nil];
    
    map<string, vector<Text*>> texts;
    
    map<string, vector<Sample>> samples = [self makeCGSamples:cgs isTraining:YES];
    texts = [self cgClassify:samples adaboost:cgvAdaBoost];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return texts;
}

#pragma mark -
#pragma mark Assistant Methods

- (void) loadGtXMLData:(BOOL) isTraining
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"--- XMLファイル読み込み開始 ---", OUTPUT, nil];
    
    XmlMaker *xml = [[XmlMaker alloc] init];
    NSURL *url = nil;
    if (isTraining) {
        if (model.trainGtXmlPath.length>0) url = [NSURL fileURLWithPath:model.trainGtXmlPath];
        else return;
    } else {
        if (model.testGtXmlPath.length>0) url = [NSURL fileURLWithPath:model.testGtXmlPath];
        else return;
    }
    
    NSString *doc = [[NSString alloc] initWithContentsOfURL:url
                                                   encoding:NSUTF8StringEncoding
                                                      error:nil];
    [xml readGtXmlAndAddData:doc];
    xmldata = [model getGtXMLData];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
}

- (void) loadAdaBoostXMLData
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"--- XMLファイル読み込み開始 ---", OUTPUT, nil];
    
    NSURL *url = nil;
    if (model.adaboostXmlPath.length>0) url = [NSURL fileURLWithPath:model.adaboostXmlPath];
    else return;
    NSString *doc = [[NSString alloc] initWithContentsOfURL:url
                                                   encoding:NSUTF8StringEncoding
                                                      error:nil];
    
    XmlMaker *xml = [[XmlMaker alloc] init];
    [xml readAdaBoostXml:doc];
    adaboostXmldata = [model getAdaBoostXMLData];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
}

inline bool CGRectAlmostContains(CGRect trect, CGRect rect)
{
    CGRect intersect = CGRectIntersection(trect, rect);
    double ratio = (double)intersect.size.width * intersect.size.height / (double)rect.size.width * rect.size.height;
    return ratio >= 0.8f;
}

- (map<string, vector<Sample>>) makeCCSamples:(const map<string, vector<Object*>>&) ccs isTraining:(BOOL) isTraining
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- ラベリング開始 --- \n", OUTPUT, nil];
    
    map<string, vector<Sample>> samples;
    
    NSArray *imagePaths;
    if (isTraining) imagePaths = model.trainImagePaths;
    else imagePaths = model.testImagePaths;
    
    for (NSString *path in imagePaths)
    {
        vector<Sample> temp;
        
        // オブジェクト 抽出
        string filepath = [path cStringUsingEncoding:NSUTF8StringEncoding];
        vector<Object*> objects = ccs.at(filepath);
        
        // filename抽出
        string filename = filepath.substr(filepath.find_last_of("/")+1);
        NSRange range = [path rangeOfString:@"/" options:NSBackwardsSearch];
        NSString *nsfilename = [path substringFromIndex:range.location+1];
        
        // Truths
        NSArray *truths = xmldata[nsfilename];
        
        // サンプル作成
        for (int i = 0; i < objects.size(); i++)
        {
            if (objects[i]->filename.compare(filename) != 0)
                continue;
            
            Object *obj = objects[i];
            Sample s(*obj);
            
            cv::Rect obj_rect = s.object.rect;
            CGRect rect = CGRectMake(obj_rect.x, obj_rect.y, obj_rect.width, obj_rect.height);
            
            bool findFlag = NO;
            for (Truth *t in truths) {
                if (CGRectContainsRect(t.rect, rect) || CGRectAlmostContains(t.rect, rect)) { // t.rectにobject.rectが含まれるなら
                    findFlag = YES;
                    break;
                }
            }
            // objのlabel付け
            if (findFlag) s.label = 1;
            else s.label = 0;
            
            temp.push_back(s);
        }
        
        vector<Sample> copy(temp);
        samples.insert(map<string, vector<Sample>>::value_type(filepath, copy));
        
    }

    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return samples;
}

- (map<string, vector<Sample>>) makeCGSamples:(const map<string, vector<Text*>>&) cgs isTraining:(BOOL)isTraining
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- ラベリング開始 --- \n", OUTPUT, nil];
    
    map<string, vector<Sample>> samples;
    
    NSArray *imagePaths;
    if (isTraining) imagePaths = model.trainImagePaths;
    else imagePaths = model.testImagePaths;
    
    for (NSString *path in imagePaths)
    {
        vector<Sample> temp;
        
        // オブジェクト 抽出
        string filepath = [path cStringUsingEncoding:NSUTF8StringEncoding];
        vector<Text*> groups = cgs.at(filepath);
        
        // filename抽出
        string filename = filepath.substr(filepath.find_last_of("/")+1);
        NSRange range = [path rangeOfString:@"/" options:NSBackwardsSearch];
        NSString *nsfilename = [path substringFromIndex:range.location+1];

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
                if (NSContainsRect(t.rect, rect)) { // TODO: 「t.rectにobject.rectが8割含まれて、4割含むなら」に変更
                    findFlag = YES;
                    break;
                }
            }
            // objのlabel付け
            if (findFlag) s.label = 1;
            else s.label = 0;
            
            temp.push_back(s);
        }
        
        vector<Sample> copy(temp);
        samples.insert(map<string, vector<Sample>>::value_type(filepath, copy));
    }
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return samples;
}

- (AdaBoost) learnFeaturesWithAdaBoost:(const map<string, vector<Sample>>&) samples
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- AdaBoost学習開始 --- \n", OUTPUT, nil];
    
    vector<Sample> trainSamples;
    
	// 全列挙
	map<string, vector<Sample>>::const_iterator itr;
    for (itr=samples.begin(); itr != samples.end(); itr++) {
        vector<Sample> temp = itr->second;
        trainSamples.insert(trainSamples.end(), temp.begin(), temp.end());
    }
    
    // ヒストグラム表示
//    for (int i = 0; i < trainSamples[0].features.size(); i++) {
//        [self showHistograms:trainSamples index:i];
//    }
    
    // ラベルデータと非ラベルデータ
//    vector<Sample>::const_iterator first = trainSamples.begin();
//    vector<Sample>::const_iterator half = trainSamples.begin() + (int)(trainSamples.size()*0.5f);
//    vector<Sample>::const_iterator end = trainSamples.end();
//    vector<Sample> labeled(first, half);
//    vector<Sample> unlabeled(half, end);
    
    // WeakClassifierを準備
    vector<WeakClassifier> classifiers;
    for (int i = 0; i < trainSamples[0].features.size(); i++) {
        WeakClassifier wc(i);
        classifiers.push_back(wc);
    }
    
    // AdaBoost learning
    AdaBoost adaboost(trainSamples, classifiers);
    
    for (int t = 0; t < trainSamples[0].features.size(); t++) {
        AdaBoostResult result = adaboost.next();
        WeakClassifier selected = result.wc;
        cout << "t:" << t << ", wc index:" << selected.featureIndex << ", alpha:" << selected.alpha << endl;
    }
    
//    [self ccClassify:trainSamples adaboost:adaboost];

    return adaboost;
}

- (map<string, vector<Object*>>) ccClassify:(map<string, vector<Sample>>) samples adaboost:(AdaBoost) adaboost
{
    map<string, vector<Object*>> components;
    
    int e = 0, E = 0, T = 0, size = 0;
    double precision = 0, recall = 0, f = 0;
    
    // 全列挙
	map<string, vector<Sample>>::iterator itr;
    for (itr=samples.begin(); itr != samples.end(); itr++) {
        
        string filepath = itr->first;
        vector<Sample> temp = itr->second;
        vector<Object*> corrects;
        
        for (int i = 0; i < temp.size(); i++)
        {
            int test = adaboost.sc.test(temp[i]);
            Object* o = &(temp[i].object);
            
            if (test>0) {
                corrects.push_back(o);
            }
            
            // 母数を計算
            E++;
            size++;
            
            // 正解個数を算出
            if (test == temp[i].label) {
                e++;
            }
        }
        
        Mat srcImage = imread(filepath);
        vector<Object*> copy(corrects);
        Draw::drawObjects(srcImage, copy);
        
        components.insert(map<string, vector<Object*>>::value_type(filepath, corrects));
    }
    
    // 母数を計算
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
    cout << "size:" << size << ", e:" << e << ", E:" << E << ", T:" << T << endl;
    cout << result << endl;
    
    return components;
}

- (map<string, vector<Text*>>) cgClassify:(map<string, vector<Sample>>) samples adaboost:(AdaBoost) adaboost
{
    map<string, vector<Text*>> texts;
    
    int e = 0, E = 0, T = 0;
    double precision = 0, recall = 0, f = 0;
    
    // 全列挙
	map<string, vector<Sample>>::iterator itr;
    for (itr=samples.begin(); itr != samples.end(); itr++) {
        
        string filepath = itr->first;
        vector<Sample> temp = itr->second;
        vector<Text*> corrects;
        
        for (int i = 0; i < temp.size(); i++)
        {
            Sample s = temp[i];
            int test = adaboost.sc.test(s);
            
            if (test == 1) {
                corrects.push_back(&s.text);
            }
            
            // 母数を計算
            E++;
            
            // 正解個数を算出
            if (test == s.label) e++;
        }
        
        texts.insert(map<string, vector<Text*>>::value_type(filepath, corrects));
    }
    
    // 母数を計算
    for (NSString *filename in [xmldata allKeys]) {
        NSArray *truths = xmldata[filename];
        T += truths.count;
    }
    
    // PとRとFを算出
    precision = (double)e / E;
    recall = (double)e / T;
    f = 2 * precision * recall / (precision + recall);
    
    NSString *result = [NSString stringWithFormat:@"\n --- 候補オブジェクト分類結果 --- \n Precision:%f, Recall:%f, F:%f", precision, recall, f];
    //    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:result, OUTPUT, nil];
    
    cout << "--- 候補オブジェクト分類結果 ---" << endl;
    cout << "size:" << samples.size() << "e:" << e << ", E:" << E << ", T:" << T << endl;
    cout << result << endl;
    
    return texts;
}

- (void) saveAdaBoostResultToXml:(NSString *) type
{
    if ([type isEqualToString:CCV])
    {
        // resultのDictionaryを作る
        adaboostResult = [[NSMutableDictionary alloc] init];
        [adaboostResult setObject:model.trainDir forKey:@"dataset"];
        
        
        NSMutableDictionary *ccv = [[NSMutableDictionary alloc] init];
        for (int i = 0; i < ccvAdaBoost.sc.wcs.size(); i++) {
            NSString *key = [NSString stringWithFormat:@"wc%d", i];
            NSDictionary *wc = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [NSString stringWithFormat:@"%d", ccvAdaBoost.sc.wcs[i].featureIndex],@"findex",
                                [NSString stringWithFormat:@"%f", ccvAdaBoost.sc.wcs[i].alpha], @"alpha",
                                [NSString stringWithFormat:@"%f", ccvAdaBoost.sc.wcs[i].threshold], @"threshold", nil];
            [ccv setObject:wc forKey:key];
        }
        [adaboostResult setObject:ccv forKey:CCV];
    }
    else if ([type isEqualToString:CGV])
    {
        NSMutableDictionary *cgv = [[NSMutableDictionary alloc] init];
        for (int i = 0; i < cgvAdaBoost.sc.wcs.size(); i++) {
            NSString *key = [NSString stringWithFormat:@"wc%d", i];
            NSDictionary *wc = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [NSString stringWithFormat:@"%d", cgvAdaBoost.sc.wcs[i].featureIndex],@"findex",
                                [NSString stringWithFormat:@"%f", cgvAdaBoost.sc.wcs[i].alpha], @"alpha",
                                [NSString stringWithFormat:@"%f", cgvAdaBoost.sc.wcs[i].threshold], @"threshold", nil];
            [cgv setObject:wc forKey:key];
        }
        [adaboostResult setObject:cgv forKey:CGV];
    
        // modelに保存
        [model setAdaBoostXMLData:adaboostResult];
    }
    else {
        ERROR(@"Type:%@ は不明なタイプです.", type);
    }
}


- (void) showHistograms:(vector<Sample>) samples index:(int)index
{
    int size = (int)samples.size();
    
    // 100 分割レベルで量子化
	const int binNum = 100;
	int plushist[binNum] = {0};
	int minushist[binNum] = {0};
    
    int max = 0;
    for (int i = 0; i < size; i++) {
        double value = samples[i].object.features[index];
        int v = floor(value*100);
        if (v > max) max = v;
        if (samples[i].label > 0) plushist[v]++;
        else minushist[v]++;
    }
    
    Mat plus(max+1, binNum, CV_8UC3);
    Mat minus(max+1, binNum, CV_8UC3);
    plus = CV_RGB(0, 0, 0);
    minus = CV_RGB(0, 0, 0);
    
    Scalar red = CV_RGB(256, 0, 0);
    Scalar blue = CV_RGB(0, 0, 256);
    for (int i = 0; i < binNum; i++) {
        line(plus, cv::Point(i, max), cv::Point(i, max-plushist[i]), red);
        line(minus, cv::Point(i, max), cv::Point(i, max-minushist[i]), blue);
    }
    
    Draw::draw(plus, minus);
}

#pragma mark -
#pragma mark Evaluate Methods

- (void) evaluateComponents:(vector<Object*>) components
{
    
}

- (void) evaluateTexts:(vector<Text*>)texts
{
    
}

inline void output(NSString *string)
{
    
}

@end
