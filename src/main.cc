#include "emulator.hpp"

#include "instruction_decoder.cpp"
#include "instructions.cpp"
#include "memory_mapper.cpp"
#include "emulator.cpp"
#include "ppu.cpp"

namespace logs {
void _log(Reg16 v) { _logx16((u16)v); };
void _log(str s) { _logs((const char *)s.data, s.size); }
}
void logs::_log(u8 v) { _logx8(v); }
void logs::_log(u16 v) { _logx16(v); }
void logs::_log(u32 v) { _logx32(v); }
void logs::_log(i32 v) { _logf(v); }
void logs::_log(double f) { _logf(f); }
void logs::_log(const char * s) { _logs(s, sslen(s)); }
void logs::_log(void * s) { _logp(s); }

extern "C" {
  emulator_t * WASM_EXPORT get_emulator() {return new emulator_t {}; }
  void WASM_EXPORT set_breakpoint(emulator_t * e, u16 addr) {e->debug.set_breakpoint(addr);}
  void WASM_EXPORT clear_breakpoint(emulator_t * e, u16 addr) {e->debug.clear_breakpoint(addr);}
  void WASM_EXPORT button_down(emulator_t * e, u16 button) { e->joypad.button_down((Buttons)button); }
  void WASM_EXPORT button_up(emulator_t * e, u16 button) { e->joypad.button_up((Buttons)button); }

  void WASM_EXPORT step_instruction(emulator_t * e) {
    e->printer.pc = e->decoder.pc_start;
    e->printer.decode();
    e->_runner.dump();
    e->single_step();
  }
  
  void WASM_EXPORT continue_instr(emulator_t * e) {
    e->debug.is_debugging = false;
    e->debug.is_stepping = false;
  }
  
  void WASM_EXPORT step_frame(emulator_t * e) {
    #define CLOCK_HZ 8200000
    #define FPS 60
    e->step(CLOCK_HZ / FPS);
  }

  u8* WASM_EXPORT get_rom_area(emulator_t * e, u32 len) {
    return new u8[len];
  }
  
  void WASM_EXPORT reset(emulator_t * e, u8 * cart, u32 len) {
    e->load_cart(cart, len);
    e->decoder.pc = 0;
    e->decoder.pc_start = 0;
  }
}
