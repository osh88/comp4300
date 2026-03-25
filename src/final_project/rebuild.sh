#!/bin/sh

mkdir -p build && cd build
cmake --fresh .. && cmake --build . -j8
cd ../
