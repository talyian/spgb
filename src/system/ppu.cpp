#include "ppu.hpp"
#include "mmu.hpp"

u8 rgb(u8 r, u8 g, u8 b) {
  return r * 36 + g * 6 + b;
}
u8 absolute_palette[4] = {
  rgb(5, 5, 4),
  rgb(3, 5, 2),
  rgb(1, 2, 2),
  rgb(0, 0, 0)
};
void PPU::set_display(u8 x, u8 y, u8 pixel) {
  display[y * DISPLAY_W + x] = absolute_palette[pixel % 4];
}

void PPU::tick(u16 delta) {
  LcdStatusMatch = 0;  
  line_timer += delta;
  monotonic_timer += delta;
START:
  switch (state) {
  case OAM_SCAN: // scanning oam
    if (line_timer < 0x50) {
      break;
    }
    state = VRAM_SCAN;
  case VRAM_SCAN: // scanning vram
    if (line_timer < 0xFC) {
      break;
    }
    state = HSCAN;
    if (LcdStatus.IrqHBlank) { LcdStatusMatch = 1; }
  case HSCAN: // horizontal scan
    if (line_timer < 0x1C8) {
      break;
    }
    line_timer -= 0x1C8;
    if (LineY < 144)
      scan_line(); // [0 - 144) -- scan line
    else if (LineY == 144) {
      if (LcdStatus.IrqVBlank) { LcdStatusMatch = 1; }
      push_frame();
    }
    else if (LineY == 153) {
      LineY = -1;
    }
    LineY++;
    LcdStatus.LYMatch = LineY == LineYMark;
    if (LcdStatus.LYMatch && LcdStatus.IrqLYMatch) { LcdStatusMatch = 1; }
    if (LcdStatus.IrqOAM) { LcdStatusMatch = 1; }
    state = OAM_SCAN;
    goto START;
  default:
    log("invalid PPU state", state);
    state = OAM_SCAN;
  };

  if (LcdStatusMatch - LcdStatusLastMatch == 1) { InterruptV |= 2; } // LCD
  LcdStatusLastMatch = LcdStatusMatch;
}

void PPU::push_frame() {
  frame++;
  _push_frame(0x300, display, DISPLAY_W * DISPLAY_H);
  _push_frame(0x100, mmu->VRAM, 0x800);
  _push_frame(0x101, mmu->VRAM + 0x800, 0x800);
  _push_frame(0x102, mmu->VRAM + 0x1000, 0x800);
  _push_frame(0x200, mmu->VRAM + 0x1800, 32 * 32);
  InterruptV |= 0x01; // HBLANK
}

// Given a bg coordinate between 0,0 and 32,32, return the tile specifier
u8 PPU::select_background_tile(u8 x, u8 y) {
  u8 bg_tile_source = LcdControl & 0x8;
  return mmu->VRAM[0x80 * bg_tile_source + 0x1800 + y * 32 + x];
};

u8 load_tile_pixel(u8 * tile_ptr, u8 x, u8 y) {
  u16 t = *(u16 *)tile_ptr;
  t = t >> (7 - x);
  t = t & 0x0101;
  return t | (t >> 7);
}

void PPU::scan_line() {
  // reads in LineY and writes that scanline to `display`
  u8 bg_y = LineY + ScrollY;
  u8 bg_tile_y = bg_y / 8;
  u8 bg_tile_x = ScrollX / 8;
  for(u8 tile = 0; ; tile++) {
    u8 tile_index = select_background_tile(bg_tile_x + tile, bg_tile_y);
    // 0x0000-0x0800   (A:0-128)
    // 0x0800-0x1000 (A:128-256) (B:128-256)
    // 0x1000-0x1800   (B:0-128)
    u8 * tile_data = 0, ty = bg_y % 8;
    if ((tile_index & 0x80) | (LcdControl & 0x10)) {
      tile_data = mmu->VRAM + tile_index * 16 + ty * 2;
    } else {
      tile_data = mmu->VRAM + 0x1000 + tile_index * 16 + ty * 2;
    }
    for(u8 tx = 0; tx < 8; tx++) {
      if (tile * 8 + tx >= DISPLAY_W) goto BG_DONE;
      auto pixel = load_tile_pixel(tile_data, tx, ty);
      // pixel = BgPalette << (2 * pixel) & 0x03;
      set_display(tile * 8 + tx, LineY, pixel);
    }
  }
 BG_DONE:
  // TODO: window
  
  // scan entire sprite table for overlapping sprites
  for (u8 i = 0; i < 0xA0; i += 4) {
    OamEntry oam_entry = *(OamEntry *)(mmu->OAM + i);
    u8 tile_index = oam_entry.tile;
    u8 y = LineY - oam_entry.y + 16;
    if (y >= 8) continue;
    if (oam_entry.x == 0) continue;
    if (oam_entry.x >= DISPLAY_W) continue;
    u8 flipy = oam_entry.flags.flip_y();
    u8 flipx = oam_entry.flags.flip_x();
    if (flipy) y = 7 - y;

    u8 * tile_data = mmu->VRAM + tile_index * 16 + y * 2;
    for(u8 _x = 0; _x < 8; _x++) {
      u8 screen_x = _x + oam_entry.x - 8, x;
      if (screen_x >= DISPLAY_W) continue;
      x = 7 * flipx - 2 * flipx * _x + _x;
      auto tile_pixel = load_tile_pixel(tile_data, x, y);
      if (tile_pixel)
        set_display(screen_x, LineY, tile_pixel);
    }
  }
}