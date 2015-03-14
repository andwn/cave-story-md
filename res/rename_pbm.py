import os, glob

for file in glob.glob('*.pbm'):
	os.rename(file, file[:-4] + '.bmp')