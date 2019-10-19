lib_gb.o: ../lib_gb.cpp ../platform.hpp ../base.hpp ../system/mmu.cpp \
  ../system/mmu.hpp ../system/../base.hpp ../system/io_ports.hpp \
  ../system/../platform.hpp ../system/timer.hpp ../system/cart.hpp \
  ../system/../utils/str.hpp ../system/../utils/../base.hpp \
  ../system/ppu.hpp ../system/ppu.cpp ../emulator.cpp ../emulator.hpp \
  ../debugger.hpp ../system/cpu.hpp ../instruction_runner_new.hpp \
  ../system/joypad.hpp ../debug/printer.hpp ../debug/../base.hpp \
  ../debug/../system/mmu.hpp ../data/dmg_boot.hpp \
  ../platform_windows/platform_limited.cpp \
  ../platform_windows/../emulator.hpp
main_limited.o: ../platform_windows/main_limited.cpp \
  ../platform_windows/../base.hpp ../platform_windows/../platform.hpp \
  ../platform_windows/opengl_utils.hpp
main.o: ../platform_windows/main.cpp ../platform_windows/../base.hpp \
  ../platform_windows/../emulator.hpp \
  ../platform_windows/../system/cart.hpp \
  ../platform_windows/../system/../base.hpp \
  ../platform_windows/../system/../utils/str.hpp \
  ../platform_windows/../system/../utils/../base.hpp \
  ../platform_windows/../system/../platform.hpp \
  ../platform_windows/../debugger.hpp \
  ../platform_windows/../system/cpu.hpp \
  ../platform_windows/../system/mmu.hpp \
  ../platform_windows/../system/io_ports.hpp \
  ../platform_windows/../system/timer.hpp \
  ../platform_windows/../instruction_runner_new.hpp \
  ../platform_windows/../system/joypad.hpp \
  ../platform_windows/../system/ppu.hpp \
  ../platform_windows/../debug/printer.hpp \
  ../platform_windows/../debug/../base.hpp \
  ../platform_windows/../debug/../system/mmu.hpp \
  ../platform_windows/../platform.hpp \
  ../platform_windows/opengl_utils.hpp
platform_wasm.o: ../platform_wasm.cpp ../base.hpp ../platform.hpp \
  ../emulator.hpp ../system/cart.hpp ../system/../base.hpp \
  ../system/../utils/str.hpp ../system/../utils/../base.hpp \
  ../system/../platform.hpp ../debugger.hpp ../system/cpu.hpp \
  ../system/mmu.hpp ../system/io_ports.hpp ../system/timer.hpp \
  ../instruction_runner_new.hpp ../system/joypad.hpp ../system/ppu.hpp \
  ../debug/printer.hpp ../debug/../base.hpp ../debug/../system/mmu.hpp
main.o: ../platform_linux/main.cpp ../platform_linux/../base.hpp \
  ../platform_linux/../platform.hpp ../platform_linux/../emulator.hpp \
  ../platform_linux/../system/cart.hpp \
  ../platform_linux/../system/../base.hpp \
  ../platform_linux/../system/../utils/str.hpp \
  ../platform_linux/../system/../utils/../base.hpp \
  ../platform_linux/../system/../platform.hpp \
  ../platform_linux/../debugger.hpp ../platform_linux/../system/cpu.hpp \
  ../platform_linux/../system/mmu.hpp \
  ../platform_linux/../system/io_ports.hpp \
  ../platform_linux/../system/timer.hpp \
  ../platform_linux/../instruction_runner_new.hpp \
  ../platform_linux/../system/joypad.hpp \
  ../platform_linux/../system/ppu.hpp \
  ../platform_linux/../debug/printer.hpp \
  ../platform_linux/../debug/../base.hpp \
  ../platform_linux/../debug/../system/mmu.hpp
