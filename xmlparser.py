# -*- coding: utf-8 -*-

import os
import sys
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import Element, SubElement, tostring

# 読み込み先のフォルダを取得
argvs = sys.argv
argc = len(argvs)
if (argc != 2):   # 引数が足りない場合は、その旨を表示
    print 'Usage: # python %s /dirname' % argvs[0]
    quit()         # プログラムの終了
dirname = argvs[1]

# 取得したい情報
# images = {'P1110610.JPG':[{'text':'あああ''}, {'rect':'{{1,1},{1,1}}'}], 
#			'P1110611.JPG':[{'text':'いいい''}, {'rect':'{{2,2},{2,2}}'}]}
images = {}

# ファイル読み込み
def importFiles(dirname):
	# ファイル一覧取得
	

	for filename in files:
		tree = ET.parse(os.path.abspath(os.path.dirname(__file__)) + dirname + filename)
		for image in tree.getroot():
			truths = []
			for truth in image:
				t = {'text':truth.get('text'), 'rect':truth.get('rect')}
				truths.append(t);
			images[image.get('name')] = truths


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
with open(os.path.abspath(os.path.dirname(__file__)) + '/' + outXml, 'w') as outfile:
    outfile.write(stringified)