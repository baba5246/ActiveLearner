# -*- coding: utf-8 -*-

import os
import sys


# 読み込み先のフォルダを取得
argvs = sys.argv
argc = len(argvs)
if (argc != 3):   # 引数が足りない場合は、その旨を表示
    print 'Usage: # python %s ./src_dirname .ext' % argvs[0]
    quit()         # プログラムの終了
src_dirname = argvs[1]
ext = argvs[2]

# ディレクトリ以下の画像取得
for root, dirs, files in os.walk(src_dirname):
	images = filter(lambda f: f.endswith(ext), files)
	for image in images:
		src_path = os.path.join(root, image)
		print "Delete file: " + src_path
		os.remove(src_path)
