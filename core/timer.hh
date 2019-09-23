#include "memory.hh"

struct Timer {
  u16 cycle_counter = 0;
  u16 divider_counter = 0;
  u16 last_tick_rate = -1;
  Memory &mem;
  u8 &DIV,
    &Value,
    &Limit,
    &Control;
  void Step(u8 cycles);
  Timer(Memory &mem);
};
