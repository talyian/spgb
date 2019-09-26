#pragma once
#include "base.hpp"
#include "wasm_host.hpp"

struct MemoryMapper {
  MemoryMapper(u8 * rom, u8 * ram) : rom(rom), ram(ram) {
    rom_n = rom;
    
    vram = ram;
    bg0 = (u8 (*)[32][32])(ram + 0x1800);
    bg1 = (u8 (*)[32][32])(ram + 0x1C00);

    cart_ram = ram + 0x2000;
    work_ram = ram + 0x4000;
    echo_ram = ram + 0x4000;

    oam = ram + 0xFE00;
    io_port = ram + 0xFF00;
  }
  
  bool bios_active = true;
  u8 *bios_rom = 0;
  // u8 rom [0x8000];
  u8 * rom = 0;
  u8 *rom_n = 0;

  // u8 ram[0x8000];
  u8 * ram = 0;
  u8 *vram = ram;
  u8 *tile_data;
  u8 (*bg0)[32][32], (*bg1)[32][32];
  u8 select_background_tile(u8 x, u8 y) {
    return ram[0x1800 + y * 32 + x];
  }
  u8 select_tile_pixel(u8 tile_index, u8 x, u8 y) {
    if (x >= 8 || y >= 8) { log("error"); return 0; }
    u8 * tile_ptr = &vram[tile_index * 16 + 2 * y];
    u8 t1 = tile_ptr[0];
    u8 t2 = tile_ptr[1];
    x = 7 - x;
    u8 v1 = (t1 >> x) & 1;
    u8 v2 = (t2 >> x) & 1;
    return v1 * 2 + v2;
  }

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
  u8& get_ref(u16 index) {
    return
      index < 0x100 && bios_active ? bios_rom[index] :
      index < 0x4000 ? rom[index] :
      index < 0x8000 ? rom_n[index] :
      ram[index - 0x8000];
  }
  u8 get(u16 index) { return get_ref(index); }
  void set(u16 index, u8 val) { get_ref(index) = val; }

};
