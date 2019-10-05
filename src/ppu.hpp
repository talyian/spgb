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
  static const int DISPLAY_W = 160, DISPLAY_H = 144;
  u8 display[DISPLAY_W * DISPLAY_H];
  void set_display(u8 x, u8 y, u8 pixel) {
    if (!(x < DISPLAY_W && y < DISPLAY_H)) { 
      log("display error at ", x, y);
      return;
    }
    display[y * DISPLAY_W + x] = pixel;
  }
  
  MemoryMapper * memory = 0;
  u32 * ext_timer = 0;
  u32 line_timer = 0,
    frame = 0,
    line = 0;

  // The PPU state machine - drawing is timing sensitive so we need
  // to carefully control the clocks
  enum State { OAM_SCAN, VRAM_SCAN, HSCAN } state = OAM_SCAN;

  void tick(u16 delta) {
    line = memory->get(0xFF44);
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
      if (line < 144) scan_line(); // [0 - 144) -- scan line
      if (line == 143) push_frame(); // [144]
      memory->set(0xFF44, ++line);
      if (line == 154) { line = 0; } // [144 - 154) -- overscan
      state = OAM_SCAN;
      goto START;
    default: log("invalid PPU state", state); state = OAM_SCAN;
    };
  }

  void push_frame() {
    for(u8 i = 0; i < 160; i += 4) {
      OamEntry oam_entry = *(OamEntry*)(memory->oam + i - 0x8000);
      // log("oam", i / 4, oam_entry.x, oam_entry.y, oam_entry.tile);
    }
    _push_frame(0x300, display, DISPLAY_W * DISPLAY_H);
    _push_frame(0x200, memory->bg0, 32 * 32);
    _push_frame(0x100, memory->vram, 0x800);
    _push_frame(0x101, memory->vram + 0x800, 0x800);
    _push_frame(0x102, memory->vram + 0x1000, 0x800);
    memory->get_ref(0xFF0F) |= 0x01; 
  }
  void scan_line() {
    // read background line by LY + SCY
    u8 bg_y_offset = memory->get(0xFF42);
    // read background col with SCX
    u8 bg_x_offset = memory->get(0xFF43);
    
    for(u8 screen_x = 0; screen_x < DISPLAY_W; screen_x++) {
      // draw background
      u8 bg_x = screen_x + bg_x_offset;
      u8 bg_y = line + bg_y_offset;
      u8 bg_tile_x = bg_x / 8;
      u8 bg_tile_y = bg_y / 8;

      u8 pixel = 0;
      {
        u8 tile_index = memory->select_background_tile(bg_tile_x, bg_tile_y);
        u8 tile_pixel = memory->select_tile_pixel(tile_index, bg_x % 8, bg_y % 8);
        pixel = tile_pixel & 0x3;
      }

      // draw foreground
      // TODO: better way of scanning sprites
      for(u8 i = 0; i < 0xA0; i += 4) {
        OamEntry oam_entry = *(OamEntry*)(memory->oam + i);
        u8 y = line - oam_entry.y + 16;
        u8 x = screen_x - oam_entry.x + 8;

        if (0 <= y && y < 8) {
          if (x < 8) {
            u8 tile_index = oam_entry.tile;
            if (oam_entry.flags.flip_x()) x = 7 - x;
            if (oam_entry.flags.flip_y()) y = 7 - y; 
            auto tile_pixel = memory->select_tile_pixel(tile_index, x, y);
            if (tile_pixel)
              pixel = tile_pixel;
          }
        }
      }
      set_display(screen_x, line, pixel);
    }
  }
};