
#import "Processor.h"
#import "NSImage+OpenCV.h"
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

        output = [NSString stringWithFormat:@"\n---- Filename:%@, Components:%ld ----", path, objects.size()];
        LOG(@"%@", output);
        [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    }
    
    return ccs;
}

- (map<string, vector<Object*>>) trainCCV:(const map<string, vector<Object*>>&) ccs
{
    NSString *output = @"\n --- オブジェクト学習開始 --- \n";
    LOG(@"%@", output);
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    
    map<string, vector<Object*>> components;

    map<string, vector<Sample>> samples = [self makeCCSamples:ccs isTraining:YES];
    AdaBoost adaboost = [self learnFeaturesWithAdaBoost:samples];
    ccvAdaBoost = adaboost;
    [self saveAdaBoostResultToXml:CCV];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    
    components = [self ccClassify:samples adaboost:ccvAdaBoost];
    return  components;
}

- (map<string, vector<Text*>>) trainCGD:(map<string, vector<Object*>>&) components
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

- (map<string, vector<Text*>>) trainCGV:(map<string, vector<Text*>>&) cgs
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
        
        NSString *output = [NSString stringWithFormat:@"---- Filename:%@, Samples:%ld ----", path, objects.size()];
        [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    }
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return ccs;
}

- (map<string, vector<Object*>>) testCCV:(map<string, vector<Object*>>&) ccs
{
    [n sendNotification:CONSOLE_OUTPUT
         objectsAndKeys:@"\n --- オブジェクト分類開始 --- \n", OUTPUT, nil];
    
    map<string, vector<Object*>> components;
    
    map<string, vector<Sample>> samples = [self makeCCSamples:ccs isTraining:NO];
    components = [self ccClassify:samples adaboost:ccvAdaBoost];
    
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"OK", OUTPUT, nil];
    return  components;
}

- (map<string, vector<Text*>>) testCGD:(map<string, vector<Object*>>&) components
{
    NSString *output = @"\n --- グループ抽出開始 --- \n";
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:output, OUTPUT, nil];
    
    map<string, vector<Text*>> cgs;
    
    // Grouping
    for (NSString *path in model.testImagePaths) {
        
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

- (map<string, vector<Text*>>) testCGV:(map<string, vector<Text*>>&) cgs
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- グループ分類開始 --- \n", OUTPUT, nil];
    
    map<string, vector<Text*>> texts;
    
    map<string, vector<Sample>> samples = [self makeCGSamples:cgs isTraining:NO];
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

inline bool CGRectGroupContains(CGRect trect, CGRect rect)
{
    CGRect intersect = CGRectIntersection(trect, rect);
    double iarea = intersect.size.width * intersect.size.height;
    double rratio = iarea / ((double)rect.size.width * rect.size.height);
    double tratio = iarea / ((double)trect.size.width * trect.size.height);
    return (rratio >= 0.7f && tratio >= 0.6f);// || (rratio >= 0.6f && tratio >= 0.6f);
}

- (map<string, vector<Sample>>) makeCCSamples:(const map<string, vector<Object*>>&) ccs isTraining:(BOOL) isTraining
{
    [n sendNotification:CONSOLE_OUTPUT objectsAndKeys:@"\n --- ラベリング開始 --- \n", OUTPUT, nil];
    
    map<string, vector<Sample>> samples;
    
    NSArray *imagePaths;
    if (isTraining) imagePaths = model.trainImagePaths;
    else imagePaths = model.testImagePaths;
    
    long plabel = 0, mlabel = 0;
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
            Sample s(obj);
            
            cv::Rect obj_rect = s.object->rect;
            CGRect rect = CGRectMake(obj_rect.x, obj_rect.y, obj_rect.width, obj_rect.height);
            
            bool findFlag = NO;
            for (Truth *t in truths) {
                if (CGRectAlmostContains(t.rect, rect)) { // t.rectにobject.rectが含まれるなら
                    findFlag = YES;
                    break;
                }
            }
            // objのlabel付け
            if (findFlag) {
                s.label = 1;
                plabel++;
            } else {
                s.label = 0;
                mlabel++;
            }
            
            temp.push_back(s);
        }
        
        vector<Sample> copy(temp);
        samples.insert(map<string, vector<Sample>>::value_type(filepath, copy));
        
    }
    
    LOG(@"\n --- サンプル抽出結果 Total:%ld（正：%ld, 負：%ld）--- \n", plabel+mlabel, plabel, mlabel);
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
    
    long plabel = 0, mlabel = 0;
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
            Sample s(text);
            
            cv::Rect obj_rect = s.text->rect;
            CGRect rect = CGRectMake(obj_rect.x, obj_rect.y, obj_rect.width, obj_rect.height);
            
            bool findFlag = NO;
            NSArray *truths = xmldata[nsfilename];
            for (Truth *t in truths) {
                if (CGRectGroupContains(t.rect, rect)) {
                    findFlag = YES;
                    break;
                }
            }
            // objのlabel付け
            if (findFlag) {
                s.label = 1;
                plabel++;
            } else {
                s.label = 0;
                mlabel++;
            }
            temp.push_back(s);
        }
        
        vector<Sample> copy(temp);
        Mat src = imread(filepath);
        [self outputImage:Draw::drawSamples(src, temp)];
        samples.insert(map<string, vector<Sample>>::value_type(filepath, copy));
    }
    
    LOG(@"\n --- サンプル抽出結果 Total:%ld（正：%ld, 負：%ld）--- \n", plabel+mlabel, plabel, mlabel);
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
    if (trainSamples[0].features.size() > 12) [self showHistograms:trainSamples index:13];
    
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

- (map<string, vector<Object*>>) ccClassify:(const map<string, vector<Sample>>&) samples adaboost:(AdaBoost) adaboost
{
    map<string, vector<Object*>> components;
    
    int e = 0, E = 0, T = 0, size = 0;
    double precision = 0, recall = 0, f = 0;
    
    // 全列挙
    map<string, vector<Sample>>::const_iterator itr;
    for (itr=samples.begin(); itr != samples.end(); itr++) {
        
        string filepath = itr->first;
        vector<Sample> temp = itr->second;
        vector<Object*> extracts;
        extracts.reserve(temp.size());
        
        for (int i = 0; i < temp.size(); i++)
        {
            int test = adaboost.sc.test(temp[i]);
            if (test>0) {
                extracts.push_back(temp[i].object);
            }
            
            // 母数を計算
            E++;
            size++;
            
            // 正解個数を算出
            if (test == temp[i].label) {
                e++;
            }
        }
        
        Mat src = imread(filepath);
//        [self outputImage:Draw::drawObjects(src, extracts)];
        
        components.insert(map<string, vector<Object*>>::value_type(filepath, vector<Object*>(extracts)));
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
    cout << "precision:" << precision << ", recall:" << recall << ", f:" << f << endl;
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
        vector<Text*> extracts;
        extracts.reserve(temp.size());
        
        for (int i = 0; i < temp.size(); i++)
        {
            int test = adaboost.sc.test(temp[i]);
            
            if (test == 1) {
                extracts.push_back(temp[i].text);
            }
            
            // 母数を計算
            E++;
            
            // 正解個数を算出
            if (test == temp[i].label) e++;
        }
        
        Mat src = imread(filepath);
        [self outputImage:Draw::drawTexts(src, extracts)];
        texts.insert(map<string, vector<Text*>>::value_type(filepath, vector<Text*>(extracts)));
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
    cout << "precision:" << precision << ", recall:" << recall << ", f:" << f << endl;
    cout << result << endl;
    
    return texts;
}

- (void) saveAdaBoostResultToXml:(NSString *) type
{
    if ([type isEqualToString:CCV])
    {
        // resultのDictionaryを作る
        adaboostResult = [[NSMutableDictionary alloc] init];
        [adaboostResult setObject:model.trainDir forKey:DATASET];
        
        
        NSMutableDictionary *ccv = [[NSMutableDictionary alloc] init];
        for (int i = 0; i < ccvAdaBoost.sc.wcs.size(); i++) {
            NSString *key = [NSString stringWithFormat:@"wc%d", i];
            NSDictionary *wc = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [NSString stringWithFormat:@"%d", ccvAdaBoost.sc.wcs[i].featureIndex],FINDEX,
                                [NSString stringWithFormat:@"%f", ccvAdaBoost.sc.wcs[i].alpha], ALPHA,
                                [NSString stringWithFormat:@"%f", ccvAdaBoost.sc.wcs[i].threshold], THRESHOLD, nil];
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
                                [NSString stringWithFormat:@"%d", cgvAdaBoost.sc.wcs[i].featureIndex],FINDEX,
                                [NSString stringWithFormat:@"%f", cgvAdaBoost.sc.wcs[i].alpha], ALPHA,
                                [NSString stringWithFormat:@"%f", cgvAdaBoost.sc.wcs[i].threshold], THRESHOLD, nil];
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
        double value = samples[i].features[index];
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

- (void) outputImage:(const Mat&)src
{
    NSImage *image = [NSImage imageWithCVMat:src];
    NSData *data = [image TIFFRepresentation];
    NSBitmapImageRep *bitmapImageRep = [NSBitmapImageRep imageRepWithData:data];
    NSDictionary *properties = [[NSDictionary alloc]
                                initWithObjectsAndKeys:[NSNumber numberWithBool:NO], NSImageInterlaced, nil];
    data = [bitmapImageRep representationUsingType:NSJPEGFileType
                                        properties:properties];
    [n sendNotification:IMAGE_OUTPUT objectsAndKeys:data, IMAGE_DATA, nil];
    waitKey(DRAW_WAIT_TIME);
}

- (void) checkSamplesId:(map<string,vector<Sample>>) samples
{
    map<string, vector<Sample>>::iterator itr;
    for (itr = samples.begin(); itr != samples.end(); itr++) {
        cout << "path:" << itr->first << endl;
        vector<Sample> temp = itr->second;
        vector<Sample>::iterator citr;
        for(citr = temp.begin(); citr != temp.end(); citr++) {
            cout << "ID:" << citr->object->ID << endl;
        }
    }
}

- (void) checkObjectsId:(map<string,vector<Object*>>) components
{
    map<string, vector<Object*>>::iterator itr;
    for (itr = components.begin(); itr != components.end(); itr++) {
        cout << "path:" << itr->first << endl;
        vector<Object*> temp = itr->second;
        for(int i = 0; i < temp.size(); i++) {
            cout << "ID:" << temp[i]->ID << endl;
        }
    }
}

@end
