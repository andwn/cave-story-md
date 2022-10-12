import os, os.path

def bytes_to_int(bytes):
    result = 0
    for b in bytes:
        result = (result << 8) + int(b)
    return result

def gen_mdt(fn, offset):
	with open(fn, "rb") as f:
		f.read(16)
		width = bytes_to_int(f.read(4)) >> 3
		height = bytes_to_int(f.read(4)) >> 3
	outfn = fn[:-4] + ".mdt"
	with open(outfn, "w") as f:
		f.write("input " + fn + "\n")
		f.write("output " + fn[:-4] + ".pat" + "\n")
		f.write("output2 " + fn[:-4] + ".map" + "\n")
		f.write("offset " + str(offset) + "\n")
		f.write("map 0 0 " + str(width) + " " + str(height) + "\n")

if __name__ == '__main__':
	# Credits Illustrations
	folder = "res/credits/"
	for fn in os.listdir(folder):
		if fn.startswith("ill") and fn.endswith(".png"):
			gen_mdt(folder + fn, 0x4000 + 16)
	
	# Sound Test Background
	gen_mdt("res/back/soundtest.png", 0x2000 + 16)
	
	# Moon Background
	gen_mdt("res/back/bkMoonTop.png", 0x4000 + (0xD000 >> 5))
	gen_mdt("res/back/bkMoonBottom.png", 0x4000 + (16 + 32 * 8))
	
	# Fog Background
	gen_mdt("res/back/bkFogTop.png", 0x4000 + (0xD000 >> 5))
	gen_mdt("res/back/bkFogBottom.png", 0x4000 + (16 + 32 * 8))
	