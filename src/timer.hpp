#pragma once

#include "base.hpp"

struct Timer {
  u8 DIV;   // FF04
  u16 TIMA; // FF05
  u8 TMA;   // FF06
  bool Interrupt; // a bit in FF0F

  u8 Control;
  bool enabled = 0;
  u16 speed_modifier = 1024;

  void set_control(u8 v) {
    Control = v;
    enabled = v & 4;
    speed_modifier = 1 << (2 * ((v - 1) & 3) + 4);
  }
  
  u64 monotonic_t = 0;
  u32 monoTIMA = 0;
  u64 counter_t = 0;
  void tick(u32 ticks) {
    monotonic_t += ticks;

    DIV = monotonic_t / 256;

    if (!enabled)  return;
    
    counter_t += ticks;

    if (counter_t >= speed_modifier) {
      TIMA++;
      monoTIMA++;
      counter_t -= speed_modifier;
    }
    
    if (TIMA >= 0x100) { TIMA = TMA; Interrupt = 1; }
  }
};
