import os, subprocess
from StringIO import StringIO
from zipfile import ZipFile
from urllib import urlopen

cachedir = "savegen_cache/"
outdir = "save/"

def download(name):
	print('http://cavestory.org/saves/' + name)
	url = urlopen('http://cavestory.org/saves/' + name)
	myzipfile = ZipFile(StringIO(url.read()))
	for n in myzipfile.namelist():
		print(n)
		uncompressed = myzipfile.read(n)
		outputFilename = cachedir + n
		output = open(outputFilename, 'wb')
		output.write(uncompressed)
		output.close()

if __name__ == "__main__":
	savelist = []
	for i in range(1, 106):
		savelist += ['02-{0:03}.zip'.format(i)];
	
	if not os.path.exists(cachedir):
		os.makedirs(cachedir)
		for save in savelist:
			download(save)

	if not os.path.exists(outdir):
		os.makedirs(outdir)

	for filename in os.listdir(cachedir):
		print(filename)
		subprocess.call(['./prof2sram', cachedir + filename, 
				outdir + filename[:-12] + '.blastem.sram'])
		subprocess.call(['./prof2sram', '-g', cachedir + filename, 
				outdir + filename[:-12] + '.gens.srm'])
