main.o: ../main.cc ../instruction_decoder.cpp ../base.hpp \
  ../instruction_decoder.hpp ../memory_mapper.hpp ../io_ports.hpp \
  ../timer.hpp ../cart.hpp ../str.hpp ../platform.hh \
  ../instruction_printer.hpp ../instructions.hpp \
  ../instruction_runner.hpp ../cpu.hpp ../opcodes.inc \
  ../instruction_runner.cpp ../instructions.cpp ../memory_mapper.cpp \
  ../ppu.hpp ../emulator.cpp ../emulator.hpp ../debugger.hpp \
  ../joypad.hpp ../data/dmg_boot.hpp ../ppu.cpp
platform_windows.o: ../platform_windows.cc ../base.hpp ../emulator.hpp \
  ../cart.hpp ../str.hpp ../platform.hh ../debugger.hpp \
  ../memory_mapper.hpp ../io_ports.hpp ../timer.hpp \
  ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../cpu.hpp \
  ../opcodes.inc ../joypad.hpp ../ppu.hpp
platform_linux.o: ../platform_linux.cc ../base.hpp ../platform.hh \
  ../emulator.hpp ../cart.hpp ../str.hpp ../debugger.hpp \
  ../memory_mapper.hpp ../io_ports.hpp ../timer.hpp \
  ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../cpu.hpp \
  ../opcodes.inc ../joypad.hpp ../ppu.hpp
platform_wasm.o: ../platform_wasm.cc ../base.hpp ../platform.hh \
  ../emulator.hpp ../cart.hpp ../str.hpp ../debugger.hpp \
  ../memory_mapper.hpp ../io_ports.hpp ../timer.hpp \
  ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../cpu.hpp \
  ../opcodes.inc ../joypad.hpp ../ppu.hpp
