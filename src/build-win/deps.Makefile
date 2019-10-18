main.o: ../main.cpp ../platform.hpp ../base.hpp ../system/mmu.cpp \
  ../system/mmu.hpp ../system/../base.hpp ../system/io_ports.hpp \
  ../system/../platform.hpp ../system/timer.hpp ../system/cart.hpp \
  ../system/../utils/str.hpp ../system/../utils/../base.hpp \
  ../system/ppu.hpp ../system/ppu.cpp ../emulator.cpp ../emulator.hpp \
  ../debugger.hpp ../system/cpu.hpp ../instruction_runner_new.hpp \
  ../system/joypad.hpp ../debug/printer.hpp ../debug/../base.hpp \
  ../debug/../system/mmu.hpp ../data/dmg_boot.hpp
platform_windows.o: ../platform_windows.cpp ../base.hpp ../emulator.hpp \
  ../system/cart.hpp ../system/../base.hpp ../system/../utils/str.hpp \
  ../system/../utils/../base.hpp ../system/../platform.hpp \
  ../debugger.hpp ../system/cpu.hpp ../system/mmu.hpp \
  ../system/io_ports.hpp ../system/timer.hpp \
  ../instruction_runner_new.hpp ../system/joypad.hpp ../system/ppu.hpp \
  ../debug/printer.hpp ../debug/../base.hpp ../debug/../system/mmu.hpp \
  ../platform.hpp ../win32_opengl.hpp
platform_linux.o: ../platform_linux.cpp ../base.hpp ../platform.hpp \
  ../emulator.hpp ../system/cart.hpp ../system/../base.hpp \
  ../system/../utils/str.hpp ../system/../utils/../base.hpp \
  ../system/../platform.hpp ../debugger.hpp ../system/cpu.hpp \
  ../system/mmu.hpp ../system/io_ports.hpp ../system/timer.hpp \
  ../instruction_runner_new.hpp ../system/joypad.hpp ../system/ppu.hpp \
  ../debug/printer.hpp ../debug/../base.hpp ../debug/../system/mmu.hpp
platform_wasm.o: ../platform_wasm.cpp ../base.hpp ../platform.hpp \
  ../emulator.hpp ../system/cart.hpp ../system/../base.hpp \
  ../system/../utils/str.hpp ../system/../utils/../base.hpp \
  ../system/../platform.hpp ../debugger.hpp ../system/cpu.hpp \
  ../system/mmu.hpp ../system/io_ports.hpp ../system/timer.hpp \
  ../instruction_runner_new.hpp ../system/joypad.hpp ../system/ppu.hpp \
  ../debug/printer.hpp ../debug/../base.hpp ../debug/../system/mmu.hpp
