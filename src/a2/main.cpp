// Darwin:
// clang++ -std=c++20 -I ../sfml-darwin/include -L ../sfml-darwin/lib -lsfml-window -lsfml-audio -lsfml-system -lsfml-graphics -rpath "\$ORIGIN/../../sfml-darwin/lib" *.cpp && ./a.out
//
// Linux:
// clang++ -std=c++20 -I ../../sfml-linux/include -L ../../sfml-linux/lib -lsfml-window -lsfml-audio -lsfml-system -lsfml-graphics -rpath "\$ORIGIN/../../sfml-linux/lib" *.cpp && ./a.out

#include "Game.h"

int main() {
    Game g("config.txt");
    g.run();
}
