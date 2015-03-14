import wave
import contextlib
from glob import glob

for fname in glob('*.wav'):
	with contextlib.closing(wave.open(fname,'r')) as f:
		frames = f.getnframes()
		rate = f.getframerate()
		duration = int(frames / 22050.0 * 14000.0 + 0.999)
		print fname + ", " + str(duration)