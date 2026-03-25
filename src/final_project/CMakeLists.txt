# mkdir build && cd build
# cmake .. && cmake --build . -j8

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)

# Names your project.
# You should always list languages; if not specified, CMake assumes a C++ and C mixed project.
project(game VERSION 1.0 LANGUAGES CXX)

add_library(sfml-audio SHARED IMPORTED)
set_target_properties(sfml-audio PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/deps/linux/lib/libsfml-audio.so")

add_library(sfml-graphics SHARED IMPORTED)
set_target_properties(sfml-graphics PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/deps/linux/lib/libsfml-graphics.so")

add_library(sfml-network SHARED IMPORTED)
set_target_properties(sfml-network PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/deps/linux/lib/libsfml-network.so")

add_library(sfml-system SHARED IMPORTED)
set_target_properties(sfml-system PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/deps/linux/lib/libsfml-system.so")

add_library(sfml-window SHARED IMPORTED)
set_target_properties(sfml-window PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/deps/linux/lib/libsfml-window.so")

find_package(OpenGL REQUIRED)
find_package(LibLZMA REQUIRED) # sudo apt install liblzma-dev

# ImGui begin
#file(GLOB IMGUI_CORE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui/*.cpp")
#file(GLOB IMGUI_CORE_HDR "${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui/*.h")
#add_library(ImGui STATIC ${IMGUI_CORE_SRC} ${IMGUI_CORE_HDR})
#target_include_directories(ImGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui)
#
#set(IMGUI_BACKENDS_SRC
#    # deps/src/imgui/backends/imgui_impl_glfw.cpp
#    "${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui/backends/imgui_impl_opengl3.cpp"
#)
# ImGui end

file(GLOB SOURCES CONFIGURE_DEPENDS
    "deps/src/imgui/*.cpp"
    "deps/src/imgui-sfml/*.cpp"
    "deps/src/imgui/backends/imgui_impl_opengl3.cpp"
    "deps/src/imgui/misc/cpp/*.cpp"
    "src/*.cpp"
)

# Add an executable target using the collected sources
# add_executable(game ${SOURCES} ${IMGUI_BACKENDS_SRC})
add_executable(game ${SOURCES})

# Set C++20 standard for a specific target
# set_target_properties(game PROPERTIES
#   CXX_STANDARD 20
#   CXX_STANDARD_REQUIRED ON
#   CXX_EXTENSIONS OFF
# )

target_include_directories(game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui/misc/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/src/imgui-sfml
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/linux/include
    ${LIBLZMA_INCLUDE_DIRS}
)

target_link_libraries(game PRIVATE sfml-audio sfml-graphics sfml-system sfml-window OpenGL::GL ${LIBLZMA_LIBRARIES})
