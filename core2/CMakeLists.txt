project(gb_emulator)
cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_COMPILER clang++-8)
set(CMAKE_C_COMPILER   clang++-8)

if (WASM)
  set(CMAKE_LINKER       wasm-ld-8)
  set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

  add_compile_options(
    --target=wasm32
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
  add_dependencies(gb_emulator.wasm generated_instruction_decode_table.inc)
else()
  add_executable(gb_emulator_console main.cc native_host.cc)
  add_dependencies(gb_emulator_console generated_instruction_decode_table.inc)
  add_custom_target(run
    COMMAND ./gb_emulator_console
    DEPENDS gb_emulator_console)
endif()

add_custom_command(
  OUTPUT generated_instruction_decode_table.inc
  DEPENDS opcode_table.csv generate_instruction_decode_table.py
  COMMAND python generate_instruction_decode_table.py > generated_instruction_decode_table.inc)
add_custom_target(generated_instruction_decode_table.inc
  COMMAND python3 $(CMAKE_SOURCE_DIR)/generate_instruction_decode_table.py > $(CMAKE_SOURCE_DIR)/generated_instruction_decode_table.inc
  DEPENDS $(CMAKE_SOURCE_DIR)/generated_instruction_decode_table.inc)
  

