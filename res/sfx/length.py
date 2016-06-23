import wave
import contextlib
from glob import glob

for fname in glob('*.wav'):
	with contextlib.closing(wave.open(fname,'r')) as f:
		frames = f.getnframes()
		rate = f.getframerate()
		duration = int(frames)
		print(fname + ", " + str(duration))
