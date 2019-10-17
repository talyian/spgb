#pragma once

#include "base.hpp"
#include "emulator/cart.hpp"
#include "debugger.hpp"
#include "instruction_decoder.hpp"
#include "instruction_runner_new.hpp"
#include "emulator/io_ports.hpp"
#include "emulator/joypad.hpp"
#include "emulator/mmu.hpp"
#include "emulator/ppu.hpp"
#include "emulator/timer.hpp"

using RunnerDecoder = InstructionDecoderT<InstructionRunner>;
using PrinterDecoder = InstructionDecoderT<InstructionPrinter>;

struct emulator_t {
  // Hardware resources
  Cart cart{0, 0};
  IoPorts io;
  Timer timer{io};
  MemoryMapper mmu{cart, io};
  Joypad joypad{io};
  CPU cpu;
  PPU ppu{io, mmu};

  // Subsystems
  InstructionRunner _runner{cpu};
  InstructionPrinter _printer;
  RunnerDecoder decoder{_runner};
  PrinterDecoder printer{_printer};
  InstructionDasher _dasher{cpu, mmu};
  Debugger debug{&mmu, &decoder, &cpu};

  emulator_t(u8 *, u32);
  emulator_t();
  void load_cart(u8 *data, u32 len);

  u32 single_step();
  void step(i32 ticks);

  void dma_transfer(MemoryMapper *mem, u16 addr) {
    addr = addr * 0x100;
    for (int i = 0; i < 0xA0; i++) {
      mem->OAM[i] = mem->get(addr + i);
    }
  }
};
