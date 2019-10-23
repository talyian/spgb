#include "graphics.hpp"
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
    if (LcdStatus.IrqHBlank()) { LcdStatusMatch = 1; }
  case HSCAN: // horizontal scan
    if (line_timer < 0x1C8) {
      break;
    }
    line_timer -= 0x1C8;
    if (LineY < 144)
      scan_line(); // [0 - 144) -- scan line
    else if (LineY == 144) {
      if (LcdStatus.IrqVBlank()) { LcdStatusMatch = 1; }
      push_frame();
    }
    else if (LineY == 153) {
      LineY = -1;
    }
    LineY++;
    LcdStatus.LYMatch(LineY == LineYMark);
    if (LcdStatus.LYMatch() && LcdStatus.IrqLYMatch()) { LcdStatusMatch = 1; }
    if (LcdStatus.IrqOAM()) { LcdStatusMatch = 1; }
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
  _push_frame(0x100, VRAM, 0x800);
  _push_frame(0x101, VRAM + 0x800, 0x800);
  _push_frame(0x102, VRAM + 0x1000, 0x800);
  _push_frame(0x200, VRAM + 0x1800, 32 * 32);
  InterruptV |= 0x01; // HBLANK
}

// Given a bg coordinate between 0,0 and 32,32, return the tile specifier
Tile PPU::select_background_tile(u8 x, u8 y, u8 source) {
  source = source != 0;
  u16 addr = 0x400 * source + 0x1800 + y * 32 + x;
  return Tile {VRAM[addr], {VRAM2[addr]}};
};

u8 load_tile_pixel(u8 * tile_ptr, u8 x, u8) {
  u16 t = *(u16 *)tile_ptr;
  t = t >> (7 - x);
  t = t & 0x0101;
  return t | (t >> 7);
}

u16 PPU::get_tile_pixel(const Tile &tile, u8 tx, u8 ty) {
  u8 * vram_base_ptr = VRAM;
  if (tile.flags.tile_bank()) vram_base_ptr = VRAM2;
  u8 * tile_data = 0;
  if ((tile.index & 0x80) | (LcdControl & 0x10)) {
    tile_data = vram_base_ptr + tile.index * 16 + ty * 2;
  } else {
    tile_data = vram_base_ptr + 0x1000 + tile.index * 16 + ty * 2;
  }
  u8 pixel = load_tile_pixel(tile_data, tx, ty);
  return (BgPalette >> (2 * pixel)) & 0x03;
}

// TODO: this is currently a pretty naive implementation and turns up hot in profiler.
void PPU::scan_line() {
  // we're rendering at screen-Y LineY
  // for each screen x we're pulling the tile at bg: {sx + ScrollX, LineY + ScrollY}
  for(u8 sx = 0; sx < DISPLAY_W; sx++) {
    u8 bx = sx + ScrollX;
    u8 by = LineY + ScrollY;
    u8 tile_x = bx / 8;
    u8 tile_y = by / 8;
    Tile tile = select_background_tile(tile_x, tile_y, LcdControl & 0x8);
    u8 pixel = get_tile_pixel(tile, bx % 8, by % 8);
    set_display(sx, LineY, pixel);
  }

  // TODO : render Window
  if (LcdControl & 0x20)
    if (LineY >= this->WindowY) {
      u8 wy = LineY - this->WindowY;
      for(u8 sx = 0; sx < DISPLAY_W; sx++) {
        u8 wx = sx - WindowX + 7;
        if (wx >= DISPLAY_W) continue;
        u8 tile_x = wx / 8;
        u8 tile_y = wy / 8;
        Tile tile = select_background_tile(tile_x, tile_y, LcdControl & 0x40);
        u8 pixel = get_tile_pixel(tile, wx % 8, wy % 8);
        set_display(sx, LineY, pixel);
      }
    }
  
  auto render_sprite = [this](OamEntry &sprite) {
    u8 _ty = LineY - (sprite.y - 16);
    if (_ty >= 8) return;
    u8 ty = (sprite.flags.flip_y()) ? 7 - _ty : _ty;
    for(u8 _tx = 0; _tx < 8; _tx++) {
      u8 sx = _tx + sprite.x - 8;
      u8 tx = sprite.flags.flip_x() ? 7 - _tx : _tx;
      if (sx < DISPLAY_W) {
        u8 * tile_data = VRAM + sprite.tile * 16 + ty * 2;
        u8 raw_pixel = load_tile_pixel(tile_data, tx, ty);
        if (!raw_pixel) continue;
        u8 pixel = sprite.flags.dmg_pal() ?
          (OamPalette2 >> (2 * raw_pixel)) :
          (OamPalette1 >> (2 * raw_pixel));
        set_display(sx, LineY, pixel);
      }
    }
  };

  if (LcdControl & 4) { // double-height sprite
    for (u8 i = 0; i < 0xA0; i += 4) {
       OamEntry sprite1 = *(OamEntry *)(OAM + i);
       OamEntry sprite2 = sprite1;
       if (sprite1.flags.flip_y()) {
         sprite1.tile &= ~1;
         sprite2.tile |= 1;
         sprite1.y += 8;
       } else {
         sprite1.tile &= ~1;
         sprite2.tile |= 1;
         sprite2.y += 8;
       }
       render_sprite(sprite1);
       render_sprite(sprite2);
    }
  }
  else {
    for (u8 i = 0; i < 0xA0; i += 4) {
      render_sprite(*(OamEntry *)(OAM + i));
    }
  }
}
