main.o: ../main.cc ../emulator.hpp ../base.hpp ../io_ports.hpp \
  ../memory_mapper.hpp ../timer.hpp ../wasm_host.hpp ../str.hpp \
  ../cart.hpp ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../cpu.hpp \
  ../opcodes.inc ../ppu.hpp ../debugger.hpp ../joypad.hpp \
  ../platform_utils.cc ../instruction_decoder.cpp ../instructions.cpp \
  ../memory_mapper.cpp ../emulator.cpp ../data/dmg_boot.hpp ../ppu.cpp
native_host.o: ../native_host.cc ../base.hpp ../wasm_host.hpp ../str.hpp \
  ../emulator.hpp ../io_ports.hpp ../memory_mapper.hpp ../timer.hpp \
  ../cart.hpp ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../cpu.hpp \
  ../opcodes.inc ../ppu.hpp ../debugger.hpp ../joypad.hpp
