#!/bin/bash

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
    python3 tools/snap_htmlgen.py > "$1/index.html"
    echo "Page generated";
else
    echo "Already up to date"
fi
