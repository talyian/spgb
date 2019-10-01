#pragma once
#include "base.hpp"
#include "memory_mapper.hpp"
#include "instruction_decoder.hpp"

using RunnerDecoder = InstructionDecoderT<InstructionRunner>;

struct Debugger {
  Debugger(
    MemoryMapper * mmu,
    RunnerDecoder * decoder,
    CPU * cpu) : mmu(mmu), decoder(decoder), cpu(cpu) { }
  MemoryMapper * mmu;
  RunnerDecoder * decoder;
  CPU * cpu;
  u16 pc = 0;

  bool is_debugging = false;
  bool is_stepping = false;
  u16 breakpoints[64];
  u16 break_temp = 0;
  u8 break_n = 0;

  void set_breakpoint(u16 v) { breakpoints[break_n++] = v; }

  void clear_breakpoint(u16 v) {
    for(int i=0; i<break_n; i++)
      if (breakpoints[i] == v)
        breakpoints[i] = breakpoints[--break_n];
  }

  
  int step() {
    if (!mmu->bios_active && !is_stepping) {
      for(int i=0; i<break_n; i++) {
        if (breakpoints[i] == decoder->pc) {
          is_debugging = true;
          log("======================================== DEBUG ======");
          return 0;
        }
      }
    }

    if (break_temp && break_temp == decoder->pc) {
      is_debugging = true;
      break_temp = 0;
      return 0;
    }

    return 0;
  }
};
