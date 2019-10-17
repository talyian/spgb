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

  struct State {
    enum { RUN = 0, PAUSE, STEP, RUN_TO, RUN_TO_RET } type;
    union {
      u16 addr;
    };
  } state {};
  
  u16 breakpoints[64];
  u8 break_n = 0;
  void set_breakpoint(u16 v) { breakpoints[break_n++] = v; }

  void clear_breakpoint(u16 v) {
    for(int i=break_n; i-- > 0;)
      if (breakpoints[i] == v)
        breakpoints[i] = breakpoints[--break_n];
  }

  int step() {
    // check if current pc matches any breakpoints
    switch(state.type) {
    case State::PAUSE:
      break;
    case State::STEP:
      state.type = State::PAUSE;
      break;
    case State::RUN:
      if (mmu->BiosLock)
        for(int i = 0; i < break_n; i++)
          if (breakpoints[i] == decoder->pc) {
            state.type = State::PAUSE;
            break;
          }
      break;
    case State::RUN_TO:
      if (state.addr == decoder->pc)
        state.type = State::PAUSE;
      break;
    case State::RUN_TO_RET:
      break;
    }
    return 0;
  }
};
