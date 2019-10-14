main.o: ../main.cc ../emulator.hpp ../base.hpp ../cart.hpp ../str.hpp \
  ../platform.hh ../debugger.hpp ../memory_mapper.hpp ../io_ports.hpp \
  ../timer.hpp ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../cpu.hpp \
  ../opcodes.inc ../joypad.hpp ../ppu.hpp ../platform_shared.cc \
  ../instruction_decoder.cpp ../instructions.cpp ../memory_mapper.cpp \
  ../emulator.cpp ../data/dmg_boot.hpp ../ppu.cpp
native_host.o: ../native_host.cc ../base.hpp ../platform.hh \
  ../emulator.hpp ../cart.hpp ../str.hpp ../debugger.hpp \
  ../memory_mapper.hpp ../io_ports.hpp ../timer.hpp \
  ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../cpu.hpp \
  ../opcodes.inc ../joypad.hpp ../ppu.hpp
platform_wasm.o: ../platform_wasm.cc ../base.hpp ../platform.hh
