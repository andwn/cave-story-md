from pathlib import Path
import os
import shutil
import subprocess


if __name__ == "__main__":
    # files = []
    # for folder in ['.', 'Eggs', 'Hell', 'Maze', 'Mimi', 'White']:
    #    files += [folder + '/' + each for each in os.listdir(folder) if each.endswith('.bmp')]
    #    files += [folder + '/' + each for each in os.listdir(folder) if each.endswith('.png')]

    for file in ['PrtCent.bmp', 'PrtEggX1.bmp', 'PrtEggX2.bmp', 'PrtJail.bmp', 'PrtSand.bmp', 'PrtStore.bmp', 'PrtWeed.bmp']:
        shutil.rmtree('./inter', ignore_errors=True)
        os.mkdir('./inter')
        ext = Path(file).suffix
        output = Path(file).with_stem(f"{Path(file).stem}_vert")
        subprocess.run(['convert', file, '-crop', '16x16', '+adjoin', 'inter/tile_%03d' + ext])
        subprocess.run(['montage', '-mode', 'concatenate', '-tile', '1x', 'inter/tile_*' + ext, output])
