# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/mnt/d/dino-rl/build/_deps/raylib-src"
  "/mnt/d/dino-rl/build/_deps/raylib-build"
  "/mnt/d/dino-rl/build/_deps/raylib-subbuild/raylib-populate-prefix"
  "/mnt/d/dino-rl/build/_deps/raylib-subbuild/raylib-populate-prefix/tmp"
  "/mnt/d/dino-rl/build/_deps/raylib-subbuild/raylib-populate-prefix/src/raylib-populate-stamp"
  "/mnt/d/dino-rl/build/_deps/raylib-subbuild/raylib-populate-prefix/src"
  "/mnt/d/dino-rl/build/_deps/raylib-subbuild/raylib-populate-prefix/src/raylib-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/mnt/d/dino-rl/build/_deps/raylib-subbuild/raylib-populate-prefix/src/raylib-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/mnt/d/dino-rl/build/_deps/raylib-subbuild/raylib-populate-prefix/src/raylib-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
