// Darwin:
// clang++ -std=c++17 -I ../../sfml-darwin/include -L ../../sfml-darwin/lib -lsfml-window -lsfml-audio -lsfml-system -lsfml-graphics -rpath /Users/osh88/cpp/src/comp4300/sfml-darwin/lib *.cpp && ./a.out
//
// Linux:
// clang++ -std=c++20 -I ../../sfml-linux/include -L ../../sfml-linux/lib -lsfml-window -lsfml-audio -lsfml-system -lsfml-graphics -rpath /home/osh88/cpp/src/comp4300/sfml-linux/lib *.cpp && LD_LIBRARY_PATH=/home/osh88/cpp/src/comp4300/sfml-linux/lib:$LD_LIBRARY_PATH ./a.out

#include "GameEngine.h"

int main() {
    GameEngine eng("assets.txt");
    eng.run();
}
