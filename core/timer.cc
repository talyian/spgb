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

Timer::Timer(Memory &mem) :
  mem(mem), DIV(mem[0xFF04]), Value(mem[0xFF05]), Limit(mem[0xFF06]), Control(mem[0xFF07]) { }

void Timer::Step(u8 cycles) {
  if ((divider_counter += cycles) >= 0x100) { DIV++; divider_counter -= 0x100; }

  bool enabled = Control & 0x4;
  if (!enabled) return;

  u16 tick_rate = Control & 3;
  tick_rate = 2 * (((tick_rate + 3) % 4) + 2); // 10 / 4 / 6 / 8
  tick_rate = 1 << tick_rate;                  // 1024 / 16 / 64 / 256

  if (tick_rate != last_tick_rate) { last_tick_rate = tick_rate; }
  cycle_counter += cycles;
  if (cycle_counter > tick_rate) { Value += 1; cycle_counter -= tick_rate; }
  if (Value == 0) { mem[0xFF0F] |= (1 << 2); Value = Limit; }
}
