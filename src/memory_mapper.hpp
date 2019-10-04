#pragma once
#include "base.hpp"
#include "wasm_host.hpp"

struct IO {
  const static u16 DMA = 0xFF46;
};

struct MemoryMapper {
  MemoryMapper(u8 * rom, u8 * ram);
  
  bool bios_active = true;
  u8 *bios_rom = 0;
  // u8 rom [0x8000];
  u8 * rom = 0;
  u8 *rom_n = 0;

  // u8 ram[0x8000];
  u8 * ram = 0;
  u8 *vram = ram;
  u8 *tile_data;
  u8 *bg0, *bg1;
  
  u8 *cart_ram = ram + 0x2000;
  u8 *work_ram = ram + 0x4000;
  u8 *echo_ram = ram + 0x4000;

  u8 *oam = ram + 0x7E00;
  u8 *io_port = ram + 0x7F00;
  u8 error;

  u8 select_background_tile(u8 x, u8 y);
  u8 select_tile_pixel(u8 tile_index, u8 x, u8 y);
  void load_cart(u8 * cart, u32 len);
  
  u8& get_ref(u16 index) {
    return
      index < 0x100 && bios_active ? bios_rom[index] :
      index < 0x4000 ? rom[index] :
      index < 0x8000 ? rom_n[index] :
      ram[index - 0x8000];
  }
  u8 get(u16 index) { return get_ref(index); }
  void set(u16 index, u8 val) {
    get_ref(index) = val;
  }
};
