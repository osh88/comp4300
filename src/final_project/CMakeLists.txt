# mkdir build && cd build
# cmake .. && cmake --build . -j8

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# set(CMAKE_BUILD_RPATH "xx/lib/darwin")

project(game VERSION 1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CSD ${CMAKE_CURRENT_SOURCE_DIR})
set(CBD ${CMAKE_CURRENT_BINARY_DIR})

############################################
# Копируем зависимости в директорию сборки #
############################################

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CSD}/assets" "${CBD}/"
    COMMENT "Copying assets"
)

########################
# Добавляем библиотеки #
########################

add_library(sfml-audio SHARED IMPORTED)
set_target_properties(sfml-audio PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/darwin/libsfml-audio.dylib")

add_dependencies(sfml-audio copy_assets)

add_library(sfml-graphics SHARED IMPORTED)
set_target_properties(sfml-graphics PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/darwin/libsfml-graphics.dylib")

add_library(sfml-network SHARED IMPORTED)
set_target_properties(sfml-network PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/darwin/libsfml-network.dylib")

add_library(sfml-system SHARED IMPORTED)
set_target_properties(sfml-system PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/darwin/libsfml-system.dylib")

add_library(sfml-window SHARED IMPORTED)
set_target_properties(sfml-window PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/darwin/libsfml-window.dylib")

add_library(flac SHARED IMPORTED)
set_target_properties(flac PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/darwin/libFLAC.14.dylib")

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
    "src/entities/*.cpp"
    "src/scenes/*.cpp"
    "src/utils/*.cpp"
    "src/vertex/*.cpp"
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
    deps/darwin/include
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
    stdc++
    m
    flac
)

target_compile_options(game PRIVATE
  -Wall -Wextra -Werror -O3
)

set_target_properties(game PROPERTIES
  INSTALL_RPATH "@executable_path/lib/darwin"
  BUILD_WITH_INSTALL_RPATH TRUE
)
