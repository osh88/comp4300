#!/bin/sh

mkdir -p build && cd build
cmake .. && cmake --build . -j8
cd ../ && build/game
