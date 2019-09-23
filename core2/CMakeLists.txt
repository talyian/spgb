project(gb_emulator)
cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_COMPILER clang++-8)
set(CMAKE_C_COMPILER   clang++-8)
set(CMAKE_LINKER       wasm-ld-8)

set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")


add_compile_options(--target=wasm32
  -fvisibility=hidden
  -std=c++14
  -Wall
  -Wno-unused-variable
  -nostdlib)

add_link_options(
  --export-dynamic
  --no-entry
  --gc-sections
  --allow-undefined
  --import-memory)
add_executable(gb_emulator.wasm main.cc)
