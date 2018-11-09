#include "ppu.hh"

u8 PPU::bgTile(u8 x,u8 y)  {
  if (LcdControl & 0x8)
    return mem[0x9C00 + y * 0x20 + x];
  else
    return mem[0x9800 + y * 0x20 + x];
}

void PPU::RenderLine()  {
    // background
    for(u8 x = 0; x < 160; x++) {
      auto tile_x = (x + SCX) / 8;
      auto tile_y = (LY + SCY) / 8;
      auto tile_id = bgTile(tile_x, tile_y);
      auto px = (x + SCX) % 8;
      auto py = (LY + SCY) % 8;
      const Tile &t = tileData(tile_id);
      display.set(x, LY, t.get(px, py));
    }
    u8 spritecount = 0;
    // foreground
    for(u8 sp = 0; sp < 40; sp++) {
      Sprite o = mem.oam[sp];
      u8 ty = o.py - LY - 8;
      if (ty >= 8) continue;
      bool any_x = 0;
      for(u8 tx = 0; tx < 8; tx++) {
        u16 x = o.px + tx - 8;
        if (x < 160) {
          any_x = 1;
          // TODO: Priority
          // TODO: palette
          Tile * t = (Tile *)&mem[0x8000 + 16 * o.tile];
          u8 color = t->get(
            o.flags & 0x20 ? 7 - tx : tx,
            o.flags & 0x40 ? ty : 7 - ty);
          if (color) display.set(x, LY, color);
        }
      }
      spritecount += any_x;
    }
}

void PPU::RenderScreen()  {
  frames++;
  mem[0xFF0F] |= (1 << 0); // VBLANK interrupt;
  SendScreen();
}

void PPU::SendTiles()  {
    struct Message {
      u8 message_type = 'b';
      u8 scx = 0, scy = 0;
      u8 ly = 0;
      u8 bg_tile_map[0x400]; // 32 x 32 tiles on the screen
      u8 bg_tile_data[0x1000]; // each tile is 8x8x2 bites = 16 bytes, 256 possible tiles
    } __attribute__((packed)) message;
    for(u16 i = 0; i < 256; i++) {
      message.bg_tile_map[i] = i;
    }
    memcpy(&message.bg_tile_data, &mem[0x8000], 0x1000);
    display.sender.send(&message, sizeof message);
    printf("Sent message %ld bytes\n", sizeof message);
  }

void PPU::SendForeground()  {
    struct Message {
      u8 message_type = 'b';
      u8 scx = 0, scy = 0;
      u8 ly = 0;
      u8 bg_tile_map[0x400]; // 32 x 32 tiles on the screen
      u8 bg_tile_data[0x1000]; // each tile is 8x8x2 bites = 16 bytes, 256 possible tiles
    } __attribute__((packed)) message;
    memset(&message.bg_tile_map, 0, 0x400);
    for(u16 i = 0; i < 40; i++) {
      printf(
        "Sprite [%02d] %02x [%02x,%02x]\n",
        i, mem.oam[i].tile, mem.oam[i].px, mem.oam[i].py);
      message.bg_tile_map[i] = mem.oam[i].tile;
    }
    memcpy(&message.bg_tile_data, &mem[0x8000], 0x0800);
    memcpy(&message.bg_tile_data[0x800], &mem[0x8800], 0x0800);
    // memcpy(&message.bg_tile_data + 0x800, &mem[0x8800], 0x0800);
    display.sender.send(&message, sizeof message);
    printf("Sent message %ld bytes\n", sizeof message);
  }

void PPU::SendBackground() {
    struct Message {
      u8 message_type = 'b';
      u8 scx = 0, scy = 0;
      u8 ly = 0;
      u8 bg_tile_map[0x400]; // 32 x 32 tiles on the screen
      u8 bg_tile_data[0x1000]; // each tile is 8x8x2 bites = 16 bytes, 256 possible tiles
    } __attribute__((packed)) message;
    // msg[0] = 'b';
    message.scx = SCX;
    message.scy = SCY;
    message.ly = LY;
    memcpy(&message.bg_tile_map, &mem[0x9800], 0x400);
    memcpy(&message.bg_tile_data, &mem[0x8000], 0x1000);
    display.sender.send(&message, sizeof message);
    printf("Sent message %ld bytes\n", sizeof message);
  }

void PPU::setState(LcdState s) {
  LcdStatus &= ~0x3;
  LcdStatus |= (u8)s & 0x3;
  state = s;
  if ((LcdStatus & (1 << 5)) && (state == LcdState::SCAN_OAM)) mem[0xFF0F] |= (1 << 1);
  if ((LcdStatus & (1 << 4)) && (state == LcdState::VBLANK)) mem[0xFF0F] |= (1 << 1);
  if ((LcdStatus & (1 << 3)) && (state == LcdState::HBLANK)) mem[0xFF0F] |= (1 << 1);
}

int vblank_counter = 0;

void PPU::Step(uint32_t t)  {

  if ((LcdControl & (1 << 7)) == 0) return; // LCD is off
  // LCDC interrupts

    clock += t;
    vblank_counter += t;
    switch(state) {
    case LcdState::SCAN_OAM:
      if (clock >= 80) {
        clock -= 80; setState(LcdState::SCAN_VRAM);
      };
      break;
    case LcdState::SCAN_VRAM:
      if (clock >= 172) {
        clock -=172; setState(LcdState::HBLANK); RenderLine(); };
      break;
    case LcdState::HBLANK:
      if (clock >= 204) {
        clock -= 204;
        if (LY++ == 0x8F) { setState(LcdState::VBLANK); vblank_counter=0; RenderScreen(); }
        else { setState(LcdState::SCAN_OAM); }
        LcdStatus &= ~(1 << 2);
        LcdStatus |= ((LY == LYC) << 2);
        if ((LcdStatus & (1 << 6)) && (LY == LYC)) mem[0xFF0F] |= (1 << 1);
      }; break;
    case LcdState::VBLANK:
      if (clock >= 456) {
        clock -= 456;
        if (LY++ == 0x8F + 10) {
          LY = 0;
          setState(LcdState::SCAN_OAM);
        }
      }
      break;
      if (clock >= 456) {
        clock = 0;
        if (LY++ == 0x99) {
          LY = 0;
          setState(LcdState::SCAN_OAM);
        }
      }
      break;
    }
  }
