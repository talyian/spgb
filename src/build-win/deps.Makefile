main.o: ../main.cc ../emulator.hpp ../base.hpp ../memory_mapper.hpp \
  ../timer.hpp ../wasm_host.hpp ../instruction_decoder.hpp \
  ../instruction_printer.hpp ../instructions.hpp \
  ../instruction_runner.hpp ../cpu.hpp ../opcodes.inc ../ppu.hpp \
  ../debugger.hpp ../joypad.hpp ../cart.hpp ../platform_utils.cc \
  ../instruction_decoder.cpp ../instructions.cpp ../memory_mapper.cpp \
  ../emulator.cpp ../data/dmg_boot.hpp
native_host.o: ../native_host.cc ../base.hpp ../wasm_host.hpp \
  ../emulator.hpp ../memory_mapper.hpp ../timer.hpp \
  ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../cpu.hpp \
  ../opcodes.inc ../ppu.hpp ../debugger.hpp ../joypad.hpp ../cart.hpp
