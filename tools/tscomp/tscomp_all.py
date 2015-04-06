import os, subprocess

#issues = []
FNULL = open(os.devnull, 'w')

for item in os.listdir('.'):
	if not item.endswith('.tsc'):
		continue
	status = subprocess.call("tscomp " + item, stdout=FNULL, stderr=subprocess.STDOUT)
	if(status != 0):
		print "Problem with " + item

for item in os.listdir('Stage'):
	if not item.endswith('.tsc'):
		continue
	status = subprocess.call("tscomp Stage/" + item, stdout=FNULL, stderr=subprocess.STDOUT)
	if(status != 0):
		print "Problem with " + item
		
#if issues.count > 0:
#	print "There was a problem"
	