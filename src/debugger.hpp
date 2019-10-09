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
  u8 break_n = 0;
  u32 run_to_target = -1;
  
  void set_breakpoint(u16 v) { breakpoints[break_n++] = v; }

  void clear_breakpoint(u16 v) {
    for(int i=break_n; i-- > 0;)
      if (breakpoints[i] == v)
        breakpoints[i] = breakpoints[--break_n];
  }

  int step() {
    // check if current pc matches any breakpoints
    if (!is_debugging && !mmu->bios_active && !is_stepping) {
      for(int i=0; i<break_n; i++) 
        if (breakpoints[i] == decoder->pc) {
          is_debugging = true;
          break;
        }
    }
    if (!is_debugging && run_to_target == decoder->pc) {
      is_debugging = true;
      run_to_target = -1;
    }
    // convert single-step to active debug
    if (is_stepping) {
      is_debugging = true;
      is_stepping = false;
    }
    return 0;
  }
};
