#include "emulator.hpp"

#include "platform_utils.cc"
#include "instruction_decoder.cpp"
#include "instructions.cpp"
#include "memory_mapper.cpp"
#include "emulator.cpp"

namespace logs {
  void _log(reg16 v) { _logx16(v.h * 256 + v.l); };
}

extern "C" {
  emulator_t * WASM_EXPORT get_emulator() {return new emulator_t {}; }
  void WASM_EXPORT set_breakpoint(emulator_t * e, u16 addr) {e->debug.set_breakpoint(addr);}
  void WASM_EXPORT clear_breakpoint(emulator_t * e, u16 addr) {e->debug.clear_breakpoint(addr);}
  void WASM_EXPORT step_frame(emulator_t * emulator) {
  #define CLOCK_HZ 8200000
  #define FPS 60
  emulator->step(CLOCK_HZ / FPS);
  }
}
