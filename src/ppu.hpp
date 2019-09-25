#pragma once
#include "base.hpp"
#include "wasm_host.hpp"
#include "memory_mapper.hpp"

struct PPU {
  MemoryMapper * memory = 0;
  u32 * ext_timer = 0;
  u32 timer = 0;
  int line = 0;
  enum State { OAM_SCAN, VRAM_SCAN, HSCAN, HBLANK, VBLANK } state;
  void next(State &s) { s = (State)(s + 1); }
  void reset(State &s) { s = (State)0; }

  #define go_next(s)  { s = (State)(s + 1); goto START; }
  #define go_reset(s)  { s = (State)0; goto START; }
  void tick(u16 delta) {
    timer += delta;
  START:
    switch(state) {
    case OAM_SCAN: // scanning oam
      if (timer > 80) { timer -= 80; go_next(state); } else break;
    case 1: // scanning vram
      if (timer > 172) { timer -= 172; go_next(state); } else break;
    case 2: // horizontal scan
      if (timer > 204) { timer -= 204; go_next(state); } else break;
    case 3: // HBLANK
      if (++line == 0x90) { go_next(state); } else { go_reset(state); }
    case 4: // VBLANK
      if (++line == 0x9A) { go_reset(state) } else break;
    default: log("invalid PPU state", state);
    };
    (*memory)[0xFF44] = line;
  }
  #undef go_next
  #undef go_reset
};
