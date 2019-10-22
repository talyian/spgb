#pragma once
#include "../base.hpp"
#include "io_ports.hpp"
#include "../platform.hpp"

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
    bool tile_bank() const { return value & 0xF; }
    u8 cbg_pal() const { return value & 0x7; }
  } flags;
};

struct MemoryMapper;
struct PPU {
  PPU(IoPorts &io, MemoryMapper &mmu) : io(io), mmu(&mmu) {}
  IoPorts &io;
  MemoryMapper *mmu = 0;
  u32 line_timer = 0, frame = 0;

  bool LcdStatusMatch = 0, LcdStatusLastMatch = 0;
  u8 &LcdControl = io.data[0x40];
  struct STAT {
    STAT(u8 & v) : v(v) { }
    u8 &v;
    bool IrqLYMatch() { return (v >> 6) & 1; }
    bool IrqOAM() { return (v >> 5) & 1; }
    bool IrqVBlank() { return (v >> 4) & 1; }
    bool IrqHBlank() { return (v >> 3) & 1; }
    bool LYMatch() { return (v >> 2) & 1; }
    void LYMatch(bool m) {
      v &= ~(1 << 2);
      v |= m << 2;
    }
  } LcdStatus { io.data[0x41] };
  u8 &ScrollY = io.data[0x42];
  u8 &ScrollX = io.data[0x43];
  u8 &LineY = io.data[0x44];
  u8 &LineYMark = io.data[0x45];
  u8 &OamDMA = io.data[0x46];
  u8 &BgPalette = io.data[0x47];
  u8 &OamPalette1 = io.data[0x48];
  u8 &OamPalette2 = io.data[0x49];
  u8 &WindowY = io.data[0x4A];
  u8 &WindowX = io.data[0x4B];
  u8 &InterruptV = io.data[0x0F];
  // The PPU state machine - drawing is timing sensitive so we need
  // to carefully control the clocks
  enum State { OAM_SCAN, VRAM_SCAN, HSCAN } state = OAM_SCAN;
  void tick(u16 delta);
  void push_frame();
  void scan_line();

  u8 select_background_tile(u8 x, u8 y, u8 tile_map);
  u8 select_tile_pixel(u8 tile_index, u8 x, u8 y);

  static const int DISPLAY_W = 160, DISPLAY_H = 144;
  u8 display[DISPLAY_W * DISPLAY_H];
  void set_display(u8 x, u8 y, u8 pixel);

  void clear() {
    ScrollX = ScrollY = LcdControl = LineYMark = LineY = 0;
    BgPalette = OamPalette1 = OamPalette2 = 0x1B;
    WindowX = WindowY = 0;
    LcdStatusMatch = 0;
    line_timer = frame = 0;
    state = OAM_SCAN;
  }
};
