import os, subprocess

def tscomp(path):
	for item in os.listdir(path):
		if not item.endswith('.tsc'):
			continue
		out = item[:-3] + 'tsb'
		print out
		status = subprocess.call("tscomp2 " + path + item + " " + path + out)
		if(status != 0):
			print "Problem with " + item

tscomp('./Stage/')
tscomp('./')
