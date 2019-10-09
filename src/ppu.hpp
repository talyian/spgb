#pragma once
#include "base.hpp"
#include "wasm_host.hpp"
#include "memory_mapper.hpp"

struct OamEntry {
  u8 y;
  u8 x;
  u8 tile;
  struct OamFlags {
    u8 value;
    bool priority() const { return value & 0x80; }
    bool flip_y() const { return value & 0x40; }
    bool flip_x() const { return value & 0x20; }
    bool dmg_pal() const { return value & 0x10; }
  } flags;
};

struct PPU {
  MemoryMapper * memory = 0;
  u32 * ext_timer = 0;
  u32 line_timer = 0,
    frame = 0,
    line = 0;

  // The PPU state machine - drawing is timing sensitive so we need
  // to carefully control the clocks
  enum State { OAM_SCAN, VRAM_SCAN, HSCAN } state = OAM_SCAN;
  void tick(u16 delta);
  void push_frame();
  void scan_line();

  static const int DISPLAY_W = 160, DISPLAY_H = 144;
  u8 display[DISPLAY_W * DISPLAY_H];
  void set_display(u8 x, u8 y, u8 pixel);
};
