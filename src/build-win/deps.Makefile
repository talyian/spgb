main.o: ../main.cpp ../instruction_decoder.cpp ../base.hpp \
  ../instruction_decoder.hpp ../emulator/mmu.hpp \
  ../emulator/io_ports.hpp ../emulator/timer.hpp ../emulator/cart.hpp \
  ../str.hpp ../platform.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../emulator/cpu.hpp \
  ../opcodes.inc ../instruction_runner.cpp ../instructions.cpp \
  ../emulator/mmu.cpp ../emulator/ppu.hpp ../emulator/ppu.cpp \
  ../emulator.cpp ../emulator.hpp ../debugger.hpp \
  ../instruction_runner_new.hpp ../emulator/joypad.hpp \
  ../data/dmg_boot.hpp
platform_windows.o: ../platform_windows.cpp ../base.hpp ../emulator.hpp \
  ../emulator/cart.hpp ../str.hpp ../platform.hpp ../debugger.hpp \
  ../emulator/mmu.hpp ../emulator/io_ports.hpp ../emulator/timer.hpp \
  ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../emulator/cpu.hpp \
  ../opcodes.inc ../instruction_runner_new.hpp ../emulator/joypad.hpp \
  ../emulator/ppu.hpp ../win32_opengl.hpp
platform_linux.o: ../platform_linux.cpp ../base.hpp ../platform.hpp \
  ../emulator.hpp ../emulator/cart.hpp ../str.hpp ../debugger.hpp \
  ../emulator/mmu.hpp ../emulator/io_ports.hpp ../emulator/timer.hpp \
  ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../emulator/cpu.hpp \
  ../opcodes.inc ../instruction_runner_new.hpp ../emulator/joypad.hpp \
  ../emulator/ppu.hpp
platform_wasm.o: ../platform_wasm.cpp ../base.hpp ../platform.hpp \
  ../emulator.hpp ../emulator/cart.hpp ../str.hpp ../debugger.hpp \
  ../emulator/mmu.hpp ../emulator/io_ports.hpp ../emulator/timer.hpp \
  ../instruction_decoder.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../instruction_runner.hpp ../emulator/cpu.hpp \
  ../opcodes.inc ../instruction_runner_new.hpp ../emulator/joypad.hpp \
  ../emulator/ppu.hpp
