#pragma once

#include "base.hpp"
#include "system/cart.hpp"
#include "debugger.hpp"
#include "executor.hpp"
#include "system/io_ports.hpp"
#include "system/joypad.hpp"
#include "system/mmu.hpp"
#include "system/graphics.hpp"
#include "system/audio.hpp"
#include "system/timer.hpp"
#include "debug/printer.hpp"

struct emulator_t {
  // Hardware resources
  Cart cart{0, 0};
  IoPorts io;
  Timer timer{io};
  Audio audio;
  PPU ppu{io};
  MemoryMapper mmu{cart, ppu, audio, io};
  Joypad joypad{io};
  CPU cpu;

  
  Executor _executor{cpu, mmu};
  Printer _printer{mmu};
  Debugger debug{&mmu,  &cpu, &_executor.PC};

  emulator_t(u8 *, u32);
  emulator_t();
  void load_cart(u8 *data, u32 len);

  bool skip_bootrom = false;
  bool noswap = true;

  u32 ff = 0; 
  u32 single_step();
  void step(i32 ticks);

  void dma_transfer(MemoryMapper *mem, u16 addr) {
    addr = addr * 0x100;
    for (int i = 0; i < 0xA0; i++) {
      mem->OAM[i] = mem->get(addr + i);
    }
  }
};
