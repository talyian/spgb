#pragma once
#include "../base.hpp"
#include "io_ports.hpp"

// An 8-bit value used to modify the display of a tile
struct SpriteFlags {
  u8 value;
  bool priority() const { return value & 0x80; }
  bool flip_y() const { return value & 0x40; }
  bool flip_x() const { return value & 0x20; }
  bool dmg_pal() const { return value & 0x10; }
  bool tile_bank() const { return value & 0x8; }
  u8 cbg_pal() const { return value & 0x7; }
};

struct OamEntry {
  u8 y;
  u8 x;
  u8 tile;
  SpriteFlags flags;
};

struct Tile {
  u8 index;
  SpriteFlags flags;
};

struct Pixel16 { 
  u8 b0, b1;
  Pixel16() = default;
  Pixel16(u8 b0, u8 b1) : b0(b0), b1(b1) { }
  Pixel16(u8 r, u8 g, u8 b) {
    u16 v = (b << 10) + (g << 5) + r;
    b0 = v;
    b1 = v >> 8;
  }
  u8 r() { return b0 & 0x1F;  }
  u8 g() { return b0 >> 5 | ((b1 & 3) << 3); }
  u8 b() { return (b1 >> 2) & 0x1F; }
  u8 a() { return b1 >> 7; }
};

struct PPU {
  PPU(IoPorts &io) : io(io) {}
  IoPorts &io;
  u32 line_timer = 0, frame = 0;

  bool LcdStatusMatch = 0, LcdStatusLastMatch = 0;
  u8 LcdControl;
  struct STAT0 {
    u8 v;
    bool IrqLYMatch() { return (v >> 6) & 1; }
    bool IrqOAM() { return (v >> 5) & 1; }
    bool IrqVBlank() { return (v >> 4) & 1; }
    bool IrqHBlank() { return (v >> 3) & 1; }
    bool LYMatch() { return (v >> 2) & 1; }
    void LYMatch(bool m) {
      v &= ~(1 << 2);
      v |= m << 2;
    }
  } LcdStatus;
  u8 ScrollY;
  u8 ScrollX;
  u8 LineY;
  u8 LineYMark;
  u8 OamDMA;
  u8 BgPalette;
  u8 OamPalette1;
  u8 OamPalette2;
  u8 WindowY;
  u8 WindowX;
  u8 &InterruptV = io.data[0x0F];

  u8 vram_bank = 0;
  u8 VRAM[0x2000];
  u8 VRAM2[0x2000];
  u8 OAM[0x100];
  // The PPU state machine - drawing is timing sensitive so we need
  // to carefully control the clocks
  enum State { OAM_SCAN, VRAM_SCAN, HSCAN } state = OAM_SCAN;
  void tick(u16 delta);
  void push_frame();
  void scan_line();

  Tile select_background_tile(u8 x, u8 y, u8 tile_map);
  Pixel16 get_tile_pixel(const Tile &tile, u8 tx, u8 ty);

  static const int DISPLAY_W = 160, DISPLAY_H = 144;
  Pixel16 display[DISPLAY_W * DISPLAY_H];
  void set_display(u8 x, u8 y, Pixel16 p);

  u8 read(u16 addr) { return (&LcdControl)[addr]; }
  void write(u16 addr, u8 val) { (&LcdControl)[addr] = val; }
  void clear() {
    ScrollX = ScrollY = LcdControl = LineYMark = LineY = 0;
    BgPalette = OamPalette1 = OamPalette2 = 0x1B;
    WindowX = WindowY = 0;
    LcdStatusMatch = 0;
    line_timer = frame = 0;
    vram_bank = 0;
    state = OAM_SCAN;
  }

   struct CGB {
    bool enabled = true;
    struct PaletteArray {
      PaletteArray() { memset(data, 0xFF, 64); }
      u8 data[64]; // 8 palettes, 4 colors per palette, 2 bytes (rgb555) per color
      Pixel16 * pixels = (Pixel16 *)data;
      u8 addr = 0;
      u8 read() { return data[addr & 0x3F]; }
      void write(u8 value) {
        data[addr & 0x3F] = value;
        addr += addr >> 7;
        addr &= ~0x40;
      }
      Pixel16 get_color(u8 pal, u8 pixel) {
        u8 addr = pal * 8 + pixel * 2;
        return { data[addr], data[addr + 1] };
      }
    };
    PaletteArray bg_palette, spr_palette;
  } Cgb;
};
