#pragma once
#include "base.hpp"
#include "io_ports.hpp"
#include "timer.hpp"
#include "wasm_host.hpp"

struct IO {
  const static u16 JOYP = 0xFF00;
  const static u16 DMA = 0xFF46;
};

struct MemoryMapper {
  MemoryMapper(u8 * rom, u8 * ram, IoPorts &io);
  
  bool bios_active = true;
  u8 *bios_rom = 0;
  // u8 rom [0x8000];
  u8 * rom = 0;
  u8 *rom_n = 0;

  // u8 ram[0x8000];
  u8 * ram = 0;
  u8 *vram = ram;

  u8 *bg0, *bg1;
  u8 *cart_ram;
  u8 *work_ram;
  u8 *echo_ram;

  u8 *oam;
  u8 *io_port;
  u8 error;

  IoPorts &io;
  
  u8 select_background_tile(u8 x, u8 y);
  u8 select_tile_pixel(u8 tile_index, u8 x, u8 y);
  void load_cart(u8 * cart, u32 len);
  
  u8 get(u16 index) {
    if (0xFF00 <= index && index < 0xFF80)
      return io.data[index & 0xFF];
    else
      return get_ref(index);
  }

  void set(u16 index, u8 val) {
    if (0xFF00 <= index && index < 0xFF80)
      io.data[index & 0xFF] = val;
    else
      get_ref(index) = val;
  }

  void clear() {
    memset(ram + 0x7F00, 0, 0x80);
  }

private:
  u8& get_ref(u16 index) {
    return
      index < 0x100 && bios_active ? bios_rom[index] :
      index < 0x4000 ? rom[index] :
      index < 0x8000 ? rom_n[index] :
      ram[index - 0x8000];
  }
};
