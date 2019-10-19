#pragma once
#include "base.hpp"
#include "system/cpu.hpp"
#include "system/mmu.hpp"

struct Debugger {
  Debugger(
    MemoryMapper * mmu,
    CPU * cpu,
    u16 * PC) : mmu(mmu), cpu(cpu), PC(PC) { }
  MemoryMapper * mmu;
  CPU * cpu;
  u16 * PC = 0;

  struct State {
    enum { RUN = 0, PAUSE, STEP, STEP1, RUN_TO, RUN_TO_RET } type;
    union {
      u16 addr;
      u16 call_depth;
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

  bool IS_CALL(u16 addr) {
    switch(mmu->get(addr)) {
    case 0xC4:
    case 0xCC:
    case 0xCD:
    case 0xD4:
    case 0xDC: return true;
    default: return false;
    }
  }
  
  bool IS_RET(u16 addr) {
    switch(mmu->get(addr)) {
    case 0xC0:
    case 0xC8:
    case 0xC9:
    case 0xD8:
    case 0xD9: return true;
    default: return false;
    }
  }
  int step() {
    // check if current pc matches any breakpoints
    switch(state.type) {
    case State::PAUSE:
      break;
    case State::STEP:
      state.type = State::STEP1;
      break;
    case State::STEP1:
      state.type = State::PAUSE;
      break;
    case State::RUN:
      if (mmu->BiosLock)
        for(int i = 0; i < break_n; i++)
          if (breakpoints[i] == *PC) {
            state.type = State::PAUSE;
            break;
          }
      break;
    case State::RUN_TO:
      if (state.addr == *PC)
        state.type = State::PAUSE;
      break;
    case State::RUN_TO_RET:
      if (IS_CALL(*PC)) {
        log("      ", *PC, "call", mmu->get16(*PC + 1), state.call_depth);
        state.call_depth++;
      }
      if (IS_RET(*PC)) {
        log("      ", *PC, "ret", mmu->get16(*PC + 1), state.call_depth);
        state.call_depth--;
      }
      if (state.call_depth == 0) state.type = State::PAUSE;
      break;
    }
    return 0;
  }
};
