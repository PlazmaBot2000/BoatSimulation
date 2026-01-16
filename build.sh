#!/bin/bash

t=$(date +"%Y%m%d_%H%M")

SCRIPT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
dest="${SCRIPT_ROOT}/Builds/build_$t"

mkdir -p "$dest/Assets"

if [ -d "${SCRIPT_ROOT}/Assets" ]; then
    cp -r "${SCRIPT_ROOT}/Assets/"* "$dest/Assets/" 2>/dev/null || true
fi

LIBS=$(pkg-config --static --libs sdl2 SDL2_image SDL2_ttf 2>/dev/null)

if [ -z "$LIBS" ]; then
    LIBS="-lSDL2 -lSDL2_image -lSDL2_ttf"
fi

g++ *.cpp Engine/*.cpp -o "$dest/main.out" \
    -std=gnu++20 \
    -lSDL_FontCache \
    $LIBS \
    -lstdc++

if [ $? -eq 0 ]; then
    echo "----------------------------------------"
    echo "Build finished: $dest"
    echo "Executable: $dest/main.out"
else
    echo "Build FAILED!"
    exit 1
fi
