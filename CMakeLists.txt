cmake_minimum_required(VERSION 3.20)
project(DinoCudaRL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Keep Raylib build fast and minimal
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

# Module 1: Game Engine & Renderer
add_library(dino_game STATIC
    game/src/renderer.cpp
)
target_include_directories(dino_game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/game/include
)
target_link_libraries(dino_game PUBLIC raylib)

# Executable 1: Human Playable Binary
add_executable(play_human apps/play_human.cpp)
target_link_libraries(play_human PRIVATE dino_game)