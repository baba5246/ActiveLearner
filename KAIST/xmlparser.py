# -*- coding: utf-8 -*-

import os
import sys
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import Element, SubElement, tostring

# 読み込み先のフォルダを取得
argvs = sys.argv
argc = len(argvs)
if (argc != 3):   # 引数が足りない場合は、その旨を表示
    print 'Usage: # python %s ./dirname -DATASET_TYPE' % argvs[0]
    quit()         # プログラムの終了
dpath = argvs[1]
dataset_type = argvs[2]

# 取得したい情報
# images = {'P1110610.JPG':[{'text':'あああ''}, {'rect':'{{1,1},{1,1}}'}], 
#			'P1110611.JPG':[{'text':'いいい''}, {'rect':'{{2,2},{2,2}}'}]}
images = {}

# ファイル読み込み関数
def importFiles(dirpath):
	# ファイル一覧取得 
	for root, dirs, files in os.walk(dirpath):
		xmls = filter(lambda f: f.endswith(".xml"), files)
		for xml in xmls:
			print "Start import.. " + os.path.join(root, xml)

			tree = ET.parse(os.path.join(root, xml))
			# ICDAR
			if dataset_type == '-ICDAR':
				for image in tree.getroot():
					imageName = image.find("imageName")
					truths = []
					for tag_rects in image:
						for truth in tag_rects:
							t = {"text":"", "rect":"{{"+truth.get("x")+","+truth.get("y")+"},{"+truth.get("width")+","+truth.get("height")+"}}"}
							truths.append(t);
					images[imageName.text] = truths
			# KAIST
			elif dataset_type == "-KAIST":
				for image in tree.getroot():
					imageName = image.find("imageName")
					truths = []
					for word in image.find("words"):
						t = {"text":"", "rect":"{{"+word.get("x")+","+word.get("y")+"},{"+word.get("width")+","+word.get("height")+"}}"}
						truths.append(t);
					images[imageName.text] = truths
			# SVT
			elif dataset_type == "-SVT":
				for image in tree.getroot():
					imageName = image.find("imageName")
					truths = []
					for tag_rect in image.find("taggedRectangles"):
						t = {"text":tag_rect.find("tag").text, "rect":"{{"+tag_rect.get("x")+","+tag_rect.get("y")+"},{"+tag_rect.get("width")+","+tag_rect.get("height")+"}}"}
						truths.append(t);
					images[imageName.text] = truths

# ファイル読み込み
importFiles(dpath)


# 書き込みXML構造作成
root = Element('root')
for image in images:
	imageTag = SubElement(root, 'image')
	imageTag.set('name', image)
	for truth in images[image]:
		truthTag = SubElement(imageTag, 'truth')
		truthTag.set('text', truth['text'])
		truthTag.set('rect', truth['rect'])

# 文字列へ変換
stringified = tostring(root)
stringified = '<?xml version="1.0" encoding="UTF-8"?>' + stringified

#書き込み先のファイル指定
outXml = 'gt.xml'

# ファイルへ書き込み
with open(os.path.abspath(dpath) + '/' + outXml, 'w') as outfile:
    outfile.write(stringified)

