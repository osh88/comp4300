# mkdir build && cd build
# cmake .. && cmake --build . -j8

cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_RPATH "\$ORIGIN/lib/linux")

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
    "${CBD}/lib/linux/libsfml-audio.so")

add_dependencies(sfml-audio copy_assets)

add_library(sfml-graphics SHARED IMPORTED)
set_target_properties(sfml-graphics PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/linux/libsfml-graphics.so")

add_library(sfml-network SHARED IMPORTED)
set_target_properties(sfml-network PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/linux/libsfml-network.so")

add_library(sfml-system SHARED IMPORTED)
set_target_properties(sfml-system PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/linux/libsfml-system.so")

add_library(sfml-window SHARED IMPORTED)
set_target_properties(sfml-window PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/linux/libsfml-window.so")

add_library(flac SHARED IMPORTED)
set_target_properties(flac PROPERTIES IMPORTED_LOCATION
    "${CBD}/lib/linux/libFLAC.so.12")

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
    stdc++
    m
    flac
)

set_target_properties(game PROPERTIES
  INSTALL_RPATH "\$ORIGIN/lib/linux"
  BUILD_WITH_INSTALL_RPATH TRUE
)
