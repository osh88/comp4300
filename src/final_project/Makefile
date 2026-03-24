# to compile and run in one command type:
# make run

# define which compiler to use
TOOLCH := /opt/homebrew/opt/llvm@20
CXX    := $(TOOLCH)/bin/clang++
OUTPUT := game
OS     := $(shell uname)
DEPS1  := deps/src
DEPS2  := deps/mac
DEPS3  := deps/linux

# linux compiler / linker flags
# sudo apt install liblzma-dev mesa-common-dev libgl1-mesa-dev
ifeq ($(OS), Linux)
# -Wno-unused-result -Wno-deprecated-declarations
    TOOLCH    := ~/opt/llvm
    CXX       := $(TOOLCH)/bin/clang++

 	CXX_FLAGS := -O3 -std=c++20
# 	INCLUDES  := -I./src -I$(DEPS_DIR)/include -I$(IMGUI_DIR) -I$(IMGUI_SFML_DIR) -I/usr/local/include
# 	LDFLAGS   := -O3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lGL -lz -llzma
	INCLUDES  := -I./src -I$(DEPS1) -I$(DEPS1)/imgui -I$(DEPS1)/imgui-sfml -I$(DEPS3)/include -I/usr/local/include
	LDFLAGS   := -O3 -L$(DEPS3)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lGL -rpath ./lib/linux -llzma
endif

# mac osx compiler / linker flags
ifeq ($(OS), Darwin)
# -Wno-unused-result -Wno-deprecated-declarations
# -g -O0 -Wall -fexceptions
	CXX_FLAGS := -O3 -std=c++20
	INCLUDES  := -I./src -I$(DEPS1) -I$(DEPS1)/imgui -I$(DEPS1)/imgui-sfml -I$(DEPS2)/include -I/usr/local/include
	LDFLAGS   := -O3 -L$(DEPS2)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -framework OpenGL -rpath ./lib/darwin -llzma
endif

# the source files for the ecs game engine
SRC_FILES := $(wildcard src/*.cpp $(DEPS1)/*cpp $(DEPS1)/imgui/*.cpp $(DEPS1)/imgui/misc/cpp/*.cpp $(DEPS1)/imgui-sfml/*.cpp)
OBJ_FILES := $(SRC_FILES:.cpp=.o)

# Include dependency files
DEP_FILES := $(SRC_FILES:.cpp=.d)
-include $(DEP_FILES)

# all of these targets will be made if you just type make
all: $(OUTPUT)

# define the main executable requirements / command
$(OUTPUT): $(OBJ_FILES) Makefile
	$(CXX) $(OBJ_FILES) $(LDFLAGS) -o ./bin/$@

# specifies how the object files are compiled from cpp files
# -march=arm64
.cpp.o:
	$(CXX) -MMD -MP -c $(CXX_FLAGS) $(INCLUDES) $< -o $@

# typing 'make clean' will remove all intermediate build files
clean:
	rm -f $(OBJ_FILES) $(DEP_FILES) ./bin/$(OUTPUT)

# typing 'make run' will compile and run the program
run: $(OUTPUT)
	bin/$(OUTPUT)
