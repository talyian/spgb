#pragma once

#include "../base.hpp"
#include "io_ports.hpp"

struct Timer {
  Timer(IoPorts &io) : io(io),
    DIV(io.data[0x04]),
    TIMA(io.data[0x05]),
    TMA(io.data[0x06]),
    Control(io.data[0x07]),
    InterruptV(io.data[0x0F]) { }

  IoPorts &io;
  u8 &DIV;   // FF04 - divider - increases at constant 16kz
  u8 &TIMA;  // FF05 - timer - increases at 4khz/256khz/64khz/16khz
  u8 &TMA;   // FF06 - timer modulus - when TIMA wraps, reset it to TMA
  u8 &Control; // FF07
  u8 &InterruptV;

  void clear() { DIV = 0; TIMA = 0; TMA = 0; Control = 0; }
  bool enabled() { return Control & 4; }
  u16 speed_modifier() {
    return 1 << (2 * ((Control + 3) & 3) + 4);
  }
  
  u64 monotonic_t = 0; // increases at 4MHz
  u64 counter_t = 0;   // increases at 4MHz, wraps after TIMA updates

  void tick(u32 ticks) {
    monotonic_t += ticks;
    // DIV increments at 16 khz, which is 4MHz / 256
    DIV = monotonic_t / 256;

    if (enabled()) {
      counter_t += ticks;
      while (counter_t >= speed_modifier()) {
        if (TIMA == 0xFF) { TIMA = TMA; InterruptV |= 4; }
        else { TIMA++; }
        counter_t -= speed_modifier();
      }
    }
  }
};
