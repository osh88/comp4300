#!/bin/sh

# clang++ -std=c++17 -I ../../sfml/include -L ../../sfml/lib -lsfml-window -lsfml-audio -lsfml-system -lsfml-graphics -rpath ./lib/ -o bin/game src/*.cpp && cd bin && ./game && cd ../

SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CURRENT_DIR="$(pwd)"

cd "${SCRIPT_DIR}"

LD_LIBRARY_PATH="${SCRIPT_DIR}/bin/lib/linux" DYLD_LIBRARY_PATH="${SCRIPT_DIR}/bin/lib/darwin" make run -j 8

cd "${CURRENT_DIR}"
