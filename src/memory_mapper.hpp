#pragma once
#include "base.hpp"

struct MemoryMapper {
  bool bios_active = true;
  u8 *bios_rom = 0;
  u8 *cart_rom = 0;
  u8 rom [0x8000];
  u8 *rom_n = rom;

  u8 ram[0x8000];
  u8 *ram_offset = ram - 0x8000;
  u8 *vram = ram;
  u8 *bg0 = ram + 0x1800;
  u8 *bg1 = ram + 0x1c00;

  u8 *cart_ram = ram + 0x2000;
  u8 *work_ram = ram + 0x4000;
  u8 *echo_ram = ram + 0x4000;

  u8 *oam = ram + 0xFE00;
  u8 *io_port = ram + 0xFF00;
  u8 error;

  void load_cart(u8 * cart, u32 len) {
    if (len > 0x8000) len = 0x8000;
    for(u32 i=0; i < len; i++) {
      rom[i] = cart[i];
    }
  }
  u8& operator[](u16 index) {
    return
      index < 0x100 && bios_active ? bios_rom[index] :
      index < 0x4000 ? rom[index] :
      index < 0x8000 ? rom_n[index] :
      ram_offset[index];
  }
};
