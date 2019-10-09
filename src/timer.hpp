#pragma once

#include "base.hpp"
#include "io_ports.hpp"

struct Timer {
  Timer(IoPorts &io) : io(io),
    DIV(io.data[0x04]),
    TIMA(io.data[0x05]),
    TMA(io.data[0x06]),
    Control(io.data[0x07]) { }

  IoPorts &io;
  u8 &DIV;   // FF04
  u8 &TIMA; // FF05
  u8 &TMA;   // FF06
  u8 &Control; // FF07
  
  bool enabled() { return Control & 4; }
  u16 speed_modifier() { return 1 << (2 * ((Control - 1) & 3) + 4); }
  
  u64 monotonic_t = 0;
  u32 monoTIMA = 0;
  u64 counter_t = 0;
  void tick(u32 ticks) {
    monotonic_t += ticks;

    DIV = monotonic_t / 256;

    if (!(Control & 4))  return;
    
    counter_t += ticks;

    if (counter_t >= speed_modifier()) {
      if (TIMA == 0xFF) { TIMA = TMA; io.data[0x0F] |= 4; }
      else { TIMA++; }
      monoTIMA++;
      counter_t -= speed_modifier();
    }
  }
};
