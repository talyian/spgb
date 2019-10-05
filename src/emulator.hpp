#pragma once

#include "base.hpp"
#include "memory_mapper.hpp"
#include "instruction_decoder.hpp"
#include "ppu.hpp"
#include "debugger.hpp"
#include "joypad.hpp"

using RunnerDecoder = InstructionDecoderT<InstructionRunner>;
using PrinterDecoder = InstructionDecoderT<InstructionPrinter>;

struct emulator_t {
  // Hardware resources
  u8 rom[0x8000];
  u8 ram[0x8000];
  CPU cpu;
  PPU ppu;
  MemoryMapper mmu {rom, ram};

  // Subsystems
  InstructionRunner _runner{cpu};
  InstructionPrinter _printer;
  RunnerDecoder decoder {_runner};
  PrinterDecoder printer {_printer};
  Debugger debug {&mmu, &decoder, &cpu};
  Joypad joypad {mmu};
  
  emulator_t();

  u32 single_step();
  void step(i32 ticks);

  void dma_transfer(MemoryMapper * mem, u16 addr) {
    addr = addr * 0x100;
    for(int i = 0; i < 0xA0; i++) {
      mem->oam[i] = mem->get(addr + i);
    }
  }
};
