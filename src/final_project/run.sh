#!/bin/sh

mkdir -p build && cd build

if cmake .. && cmake --build . -j8; then
    cd ../
    build/game
else
    cd ../
fi
