#pragma once
#include "base.hpp"

// imports
extern "C" {
  void _logf(double v);
  void _logx8(u8 v);
  void _logx16(u16 v);
  void _logs(const char * s, u32 len);
  void _showlog();
}

// exports
#define WASM_EXPORT __attribute__((visibility("default")))
extern "C" {
  int WASM_EXPORT emu_main();
  void WASM_EXPORT load_rom(u8 * data, u32 len, int option);
  u32* WASM_EXPORT get_ppu_frame(int ticks_elapsed) ;
  void WASM_EXPORT push_button(int button);
  void WASM_EXPORT release_button(int button);
}

i32 strlen(const char * s) {
  for(i32 i = 0;; i++, s++)
    if (!*s)
      return i;
}
namespace logs {
void _log(u8 v) { _logx8(v); }
void _log(u16 v) { _logx16(v); }
void _log(i32 v) { _logf(v); }
void _log(double f) { _logf(f); }
void _log(const char * s) { _logs(s, strlen(s)); }
template<class T>
void log(T x) { _log(x); _showlog (); }
template<class T, class ... TS>
void log(T x, TS ... xs) { _log(x); log(xs...); }
}
using logs::log;
using logs::_log;
