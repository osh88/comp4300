# mkdir build && cd build
# cmake .. && cmake --build . -j8

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
project(game VERSION 1.0 LANGUAGES CXX)

########################
# Добавляем библиотеки #
########################

add_library(sfml-audio SHARED IMPORTED)
set_target_properties(sfml-audio PROPERTIES IMPORTED_LOCATION
    "${CMAKE_SOURCE_DIR}/deps/linux/lib/libsfml-audio.so")

add_library(sfml-graphics SHARED IMPORTED)
set_target_properties(sfml-graphics PROPERTIES IMPORTED_LOCATION
    "${CMAKE_SOURCE_DIR}/deps/linux/lib/libsfml-graphics.so")

add_library(sfml-network SHARED IMPORTED)
set_target_properties(sfml-network PROPERTIES IMPORTED_LOCATION
    "${CMAKE_SOURCE_DIR}/deps/linux/lib/libsfml-network.so")

add_library(sfml-system SHARED IMPORTED)
set_target_properties(sfml-system PROPERTIES IMPORTED_LOCATION
    "${CMAKE_SOURCE_DIR}/deps/linux/lib/libsfml-system.so")

add_library(sfml-window SHARED IMPORTED)
set_target_properties(sfml-window PROPERTIES IMPORTED_LOCATION
    "${CMAKE_SOURCE_DIR}/deps/linux/lib/libsfml-window.so")

find_package(OpenGL REQUIRED)
find_package(LibLZMA REQUIRED) # sudo apt install liblzma-dev

########################################
# Собираем все компилируемые исходники #
########################################

file(GLOB SOURCES CONFIGURE_DEPENDS
    "deps/src/imgui/*.cpp"
    "deps/src/imgui-sfml/*.cpp"
    "deps/src/imgui/backends/imgui_impl_opengl3.cpp"
    "deps/src/imgui/misc/cpp/*.cpp"
    "src/*.cpp"
)

# Add an executable target using the collected sources
add_executable(game ${SOURCES})

######################################
# Собираем все необходимые заголовки #
######################################

target_include_directories(game PRIVATE
    src
    deps/src
    deps/src/imgui
    deps/src/imgui/backends
    deps/src/imgui/misc/cpp
    deps/src/imgui-sfml
    deps/linux/include
    ${LIBLZMA_INCLUDE_DIRS}
)

#############################################
# Привязываем добавленные библиотеки к цели #
#############################################

target_link_libraries(game PRIVATE
    sfml-audio
    sfml-graphics
    sfml-system
    sfml-window
    OpenGL::GL
    ${LIBLZMA_LIBRARIES}
)
