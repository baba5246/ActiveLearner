# -*- coding: utf-8 -*-

import os
import sys
import shutil
import random

# 読み込み先のフォルダを取得
argvs = sys.argv
argc = len(argvs)
if (argc != 4):   # 引数が足りない場合は、その旨を表示
    print 'Usage: # python %s ./src_dirname ./dst_dirname .ext' % argvs[0]
    quit()         # プログラムの終了
src_dirname = argvs[1]
dst_dirname = argvs[2]
ext = argvs[3]

# ディレクトリ以下の画像取得
for root, dirs, files in os.walk(src_dirname):
	images = filter(lambda f: f.endswith(ext), files)
	random.shuffle(images)
	count = 0
	for image in images:
		print "Start import.. " + root
		src_path = os.path.join(root, image)
		dst_path = os.path.join(os.path.abspath(dst_dirname), image)
		shutil.copyfile(src_path, dst_path)
		if count < 100:
			count+=1
		else:
			sys.exit()


