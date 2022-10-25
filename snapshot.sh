#!/bin/bash

if [ -z "$1" ] ; then
    echo "No argument supplied"
    exit 1
fi

changed=0
git remote update && git status -uno | grep -q 'Your branch is behind' && changed=1
if [ $changed = 1 ]; then
    make clean
    git pull
    make sega
    make translate
    for fn in doukutsu-*.bin; do
        today=$(date +%Y%m%d%H%M)
        mv "$fn" "$1/${fn%.*}-${today}.${fn##*.}"
    done
    mv doukutsu-en.lst "$1/doukutsu-${today}.lst"
    echo "Build successful"
else
    echo "Already up to date"
fi

python3 tools/snap_htmlgen.py "$1"
echo "Page generated"
