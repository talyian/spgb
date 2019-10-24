#pragma once
#include "../base.hpp"

/// Public API for lib_gb

// Imports - these are required from the platform host by lib_gb
extern "C" {
  // Logging
  void spgb_logf(double v);
  void spgb_logx8(u8 v);
  void spgb_logx16(u16 v);
  void spgb_logx32(u32 v);
  void spgb_logp(void* v);
  void spgb_logs(const char * s, u32 len);
  void spgb_showlog();

  // Rendering
  void spgb_push_frame(u32 category, u8 * data, u32 len);
  // Exit Main Loop 
  void spgb_stop();
  // Serial Link
  void spgb_serial_putc(u8 v);
}

#ifdef WASM
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

// Exports - these are provided by lib_gb
enum Buttons : u8 {
  RIGHT = 0,
  LEFT,
  UP,
  DOWN,
  A,
  B,
  SELECT,
  START,
};
extern "C" {
  typedef void * Emulator;
  Emulator EXPORT spgb_create_emulator();
  void EXPORT spgb_load_cart(Emulator emu, u8 * bytes, u32 len);
  void EXPORT spgb_step(Emulator emu, u32 ticks);
  void EXPORT spgb_step_instruction(Emulator emu);
  void EXPORT spgb_step_frame(Emulator emu);
  void EXPORT spgb_button_down(Emulator emu, Buttons button);
  void EXPORT spgb_button_up(Emulator emu, Buttons button);
}
