/// Platform.hpp - a shared API for implementing different OS environments.
/// current platforms: Win32+OpenGL, WASM+WebGL, Linux Console

#pragma once
#include "base.hpp"

// Imports - these are provided by the platform
extern "C" {
  void _logf(double v);
  void _logx8(u8 v);
  void _logx16(u16 v);
  void _logx32(u32 v);
  void _logp(void* v);
  void _logs(const char * s, u32 len);
  void _showlog();
  void _push_frame(u32 category, u8 * data, u32 len);
  void _stop();
}

#ifdef WASM
#define WASM_EXPORT __attribute__((visibility("default")))
#else
#define WASM_EXPORT
#endif

extern "C" size_t sslen(const char * s);
extern "C" void *memset(void *dest, int c, size_t n) throw ();

namespace logs {
void _log(u8 v);
void _log(u16 v);
void _log(u32 v);
void _log(i32 v);
void _log(double f);
void _log(const char * s);
void _log(void* s);

template<class T>
void log(T x) { _log(x); _showlog (); }
template<class T, class ... TS>
void log(T x, TS ... xs) { _log(x); log(xs...); }
}
using logs::log;
using logs::_log;
