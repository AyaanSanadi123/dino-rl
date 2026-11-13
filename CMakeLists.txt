cmake_minimum_required(VERSION 3.15)
project(dino_rl_project)

set(CMAKE_CXX_STANDARD 17)

# 1. Build Raylib and the human play target
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

# Core game library
add_library(dino_game STATIC game/src/renderer.cpp)
target_link_libraries(dino_game PRIVATE raylib)
target_include_directories(dino_game PUBLIC game/include)

# Human player executable
add_executable(play_human apps/play_human.cpp)
target_link_libraries(play_human PRIVATE dino_game raylib)

# 2. Build the Python pybind11 module for Reinforcement Learning
find_package(Python 3 COMPONENTS Interpreter Development REQUIRED)

execute_process(
  COMMAND "${Python_EXECUTABLE}" -c "import pybind11; print(pybind11.get_cmake_dir())"
  OUTPUT_VARIABLE _pybind11_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
list(APPEND CMAKE_PREFIX_PATH "${_pybind11_dir}")
find_package(pybind11 REQUIRED)

pybind11_add_module(dino_cpp python_baseline/wrapper.cpp)
target_link_libraries(dino_cpp PRIVATE dino_game)
target_include_directories(dino_cpp PRIVATE game/include)

if(MSVC)
    target_compile_options(dino_cpp PRIVATE /O2)
else()
    target_compile_options(dino_cpp PRIVATE -O3 -Wall)
endif()
