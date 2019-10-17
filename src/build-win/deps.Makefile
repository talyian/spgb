main.o: ../main.cpp ../instruction_decoder.cpp ../base.hpp \
  ../instruction_decoder.hpp ../system/mmu.hpp ../system/io_ports.hpp \
  ../system/timer.hpp ../system/cart.hpp ../system/../base.hpp \
  ../system/../utils/str.hpp ../system/../utils/../base.hpp \
  ../system/../platform.hpp ../instruction_printer.hpp \
  ../instructions.hpp ../platform.hpp ../instruction_runner.hpp \
  ../system/cpu.hpp ../opcodes.inc ../instruction_runner.cpp \
  ../instructions.cpp ../system/mmu.cpp ../system/ppu.hpp \
  ../system/ppu.cpp ../emulator.cpp ../emulator.hpp ../debugger.hpp \
  ../instruction_runner_new.hpp ../system/joypad.hpp \
  ../data/dmg_boot.hpp
platform_windows.o: ../platform_windows.cpp ../base.hpp ../emulator.hpp \
  ../system/cart.hpp ../system/../base.hpp ../system/../utils/str.hpp \
  ../system/../utils/../base.hpp ../system/../platform.hpp \
  ../debugger.hpp ../system/mmu.hpp ../system/io_ports.hpp \
  ../system/timer.hpp ../instruction_decoder.hpp \
  ../instruction_printer.hpp ../instructions.hpp ../platform.hpp \
  ../instruction_runner.hpp ../system/cpu.hpp ../opcodes.inc \
  ../instruction_runner_new.hpp ../system/joypad.hpp ../system/ppu.hpp \
  ../win32_opengl.hpp
platform_linux.o: ../platform_linux.cpp ../base.hpp ../platform.hpp \
  ../emulator.hpp ../system/cart.hpp ../system/../base.hpp \
  ../system/../utils/str.hpp ../system/../utils/../base.hpp \
  ../system/../platform.hpp ../debugger.hpp ../system/mmu.hpp \
  ../system/io_ports.hpp ../system/timer.hpp ../instruction_decoder.hpp \
  ../instruction_printer.hpp ../instructions.hpp \
  ../instruction_runner.hpp ../system/cpu.hpp ../opcodes.inc \
  ../instruction_runner_new.hpp ../system/joypad.hpp ../system/ppu.hpp
platform_wasm.o: ../platform_wasm.cpp ../base.hpp ../platform.hpp \
  ../emulator.hpp ../system/cart.hpp ../system/../base.hpp \
  ../system/../utils/str.hpp ../system/../utils/../base.hpp \
  ../system/../platform.hpp ../debugger.hpp ../system/mmu.hpp \
  ../system/io_ports.hpp ../system/timer.hpp ../instruction_decoder.hpp \
  ../instruction_printer.hpp ../instructions.hpp \
  ../instruction_runner.hpp ../system/cpu.hpp ../opcodes.inc \
  ../instruction_runner_new.hpp ../system/joypad.hpp ../system/ppu.hpp
