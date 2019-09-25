#pragma once
#include "base.hpp"

// imports
extern "C" {
  void _logf(double v);
  void _logx8(u8 v);
  void _logx16(u16 v);
  void _logs(const char * s, u32 len);
  void _showlog();
  void _push_frame(u32 category, u8 * data, u32 len);
}

// exports
#define WASM_EXPORT __attribute__((visibility("default")))
extern "C" {
  int WASM_EXPORT emu_main();
  void WASM_EXPORT load_rom(u8 * data, u32 len, int option);
  u32* WASM_EXPORT get_ppu_frame(int ticks_elapsed) ;
  void WASM_EXPORT push_button(int button);
  void WASM_EXPORT release_button(int button);
  u8 * WASM_EXPORT get_rom_memory();
  void *memcpy(void *dest, const void *src, size_t n);
}

i32 strlen(const char * s);

namespace logs {
void _log(u8 v);
void _log(u16 v);
void _log(i32 v);
void _log(double f);
void _log(const char * s);
template<class T>
void log(T x) { _log(x); _showlog (); }
template<class T, class ... TS>
void log(T x, TS ... xs) { _log(x); log(xs...); }
}
using logs::log;
using logs::_log;
