#pragma once
#include "main.hh"
#include "memory.h"

struct Tile {
  u8 data[16];
  u8 get(u8 x, u8 y) const {
    u8 r0 = data[2 * y];
    u8 r1 = data[2 * y + 1];
    r0 = (r0 >> (7 - x)) & 1;
    r1 = (r1 >> (7 - x)) & 1;
    return r0 | (r1 << 1);
  }
};

enum class LcdState { SCAN_OAM = 2, SCAN_VRAM = 3, HBLANK = 0, VBLANK = 1 };

// Pixel Processing Unit
struct PPU {
  Memory &mem;
  uint16_t clock = 0;
  uint64_t frames = 0;
  bool bg_map;
  bool is_off = false;
  uint8_t
    &SCY,
    &SCX,
    &LY,
    &LYC,
    &LcdControl,
    &LcdStatus,
    &BGPal,
    &OPal0,
    &OPal1,
    &WY, &WX;
  Tile *Tiles0, *Tiles1;
  //// Display display;
  LcdState state = LcdState::SCAN_OAM;

  PPU(Memory &mem) :
    mem(mem),
    SCY(mem[0xFF42]),
    SCX(mem[0xFF43]),
    LY(mem[0xFF44]),
    LYC(mem[0xFF45]),
    LcdControl(mem[0xFF40]),
    LcdStatus(mem[0xFF41]),
    BGPal(mem[0xFF47]),
    OPal0(mem[0xFF48]),
    OPal1(mem[0xFF49]),
    WY(mem[0xFF4A]),
    WX(mem[0xFF4B]),
    Tiles0((Tile *)&mem[0x8000]),
    Tiles1((Tile *)&mem[0x9000])
  {

  }

  const Tile & tileData(u8 tile_id) {
    if (LcdControl & 0x10)
      return Tiles0[tile_id];
    else
      return Tiles1[(int8_t)tile_id];
  }

  u8 bgTile(u8 x, u8 y);
  void RenderLine();
  void RenderScreen();

  void SendTiles();
  void SendForeground();
  void SendBackground();
  void SendScreen() {
    //// display.send();
  }


  void PrintSprites() {
    for(u8 sp = 0; sp < 40; sp++) {
      Sprite o = mem.oam[sp];
      printf("[%02d] (%02hhx,%02hhx) - %02hhx\n",
             sp, o.py, o.px, o.tile);
    }
  }

  void setState(LcdState s);
  void Step(uint32_t t);
};
