#pragma once

#include "base.hpp"
#include "memory_mapper.hpp"
#include "instruction_decoder.hpp"
#include "ppu.hpp"

using RunnerDecoder = InstructionDecoderT<InstructionRunner>;
using PrinterDecoder = InstructionDecoderT<InstructionPrinter>;

struct emulator_t {
  u8 rom[0x8000];
  u8 ram[0x8000];
  MemoryMapper mmu;
  RunnerDecoder decoder {0};
  PrinterDecoder printer {0};
  PPU ppu {};


  bool is_debugging = false;
  bool is_stepping = false;
  u16 breakpoints[64];
  u8 break_n = 0;

  emulator_t();

  u32 single_step();
  void step(i32 ticks);

  void set_breakpoint(u16 v) { breakpoints[break_n++] = v; }

  void clear_breakpoint(u16 v) {
    for(int i=0; i<break_n; i++)
      if (breakpoints[i] == v)
        breakpoints[i] = breakpoints[--break_n];
  }
};
