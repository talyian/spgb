#include "ppu.hpp"
#include "memory_mapper.hpp"

void PPU::set_display(u8 x, u8 y, u8 pixel) {
  display[y * DISPLAY_W + x] = pixel;
}
  
void PPU::tick(u16 delta) {
  line_timer += delta;
 START:
  switch(state) {
  case OAM_SCAN: // scanning oam
    if (line_timer < 0x50) { break; }
    state = VRAM_SCAN;
  case VRAM_SCAN: // scanning vram
    if (line_timer < 0xFC) { break; }
    state = HSCAN;
  case HSCAN: // horizontal scan
    if (line_timer < 0x1C8) { break; }
    line_timer -= 0x1C8;
    if (LineY < 144) scan_line(); // [0 - 144) -- scan line
    if (LineY == 143) push_frame(); // [144]
    ++LineY;
    if (LineY == 154) { LineY = 0; } // [144 - 154) -- overscan
    state = OAM_SCAN;
    goto START;
  default: log("invalid PPU state", state); state = OAM_SCAN;
  };
}

void PPU::push_frame() {
  _push_frame(0x300, display, DISPLAY_W * DISPLAY_H);
  _push_frame(0x200, mmu->VRAM + 0x1800, 32 * 32);
  _push_frame(0x100, mmu->VRAM, 0x800);
  _push_frame(0x101, mmu->VRAM + 0x800, 0x800);
  _push_frame(0x102, mmu->VRAM + 0x1000, 0x800);
  mmu->set(0xFF0F, mmu->get(0xFF0F) | 0x01);
}

u8 select_background_tile(MemoryMapper *mmu, u8 x, u8 y) {
  // TODO: background map selection
  return mmu->VRAM[0x1800 + y * 32 + x];
};

u8 select_tile_pixel(MemoryMapper * mmu, u8 tile_index, u8 x, u8 y)  {
  // Each Tile is 8x8 pixels * 2 bits per pixel, so 16 bytes.
  // TODO: Tile Indexing mode 
  const u8 TILE_W = 8, TILE_H = 8;
  // Each 8x1 row in the tile takes 16-bits,
  // where the x column is stored in bit x and bit x + 8 
  if (x >= TILE_W || y >= TILE_H) { log("error"); return 0; }
  u8 * tile_ptr = &mmu->VRAM[2 * (tile_index * TILE_H + y)];
  u8 t1 = tile_ptr[0];
  u8 t2 = tile_ptr[1];
  x = 7 - x;
  u8 v1 = (t1 >> x) & 1;
  u8 v2 = (t2 >> x) & 1;
  return v1 * 2 + v2;
}

void PPU::scan_line() {
  for(u8 screen_x = 0; screen_x < DISPLAY_W; screen_x++) {
    // draw background
    u8 bg_x = screen_x + ScrollX;
    u8 bg_y = LineY + ScrollY;
    u8 bg_tile_x = bg_x / 8;
    u8 bg_tile_y = bg_y / 8;

    u8 pixel = 0;
    {
      u8 tile_index = select_background_tile(mmu, bg_tile_x, bg_tile_y);
      u8 tile_pixel = select_tile_pixel(mmu, tile_index, bg_x % 8, bg_y % 8);
      pixel = tile_pixel & 0x3;
    }
    
    // draw foreground
    // TODO: better way of scanning sprites
    for(u8 i = 0; i < 0xA0; i += 4) {
      OamEntry oam_entry = *(OamEntry*)(mmu->OAM + i);
      u8 y = LineY - oam_entry.y + 16;
      u8 x = screen_x - oam_entry.x + 8;
      
      if (0 <= y && y < 8) {
        if (x < 8) {
          u8 tile_index = oam_entry.tile;
          if (oam_entry.flags.flip_x()) x = 7 - x;
          if (oam_entry.flags.flip_y()) y = 7 - y; 
          auto tile_pixel = select_tile_pixel(mmu, tile_index, x, y);
          if (tile_pixel)
            pixel = tile_pixel;
        }
      }
    }
    set_display(screen_x, LineY, pixel);
  }
}