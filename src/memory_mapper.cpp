#include "memory_mapper.hpp"

MemoryMapper::MemoryMapper(u8 * rom, u8 * ram) : rom(rom), ram(ram) {
    rom_n = rom;
    vram = ram;
    bg0 = vram + 0x1800;
    bg1 = vram + 0x1c00;
    cart_ram = ram + 0x2000;
    work_ram = ram + 0x4000;
    echo_ram = ram + 0x4000;

    oam = ram + 0x7E00;
    io_port = ram + 0x7F00;

    set(0xFF42, 0);
    set(0xFF43, 0);
    set(0xFF44, 0);
    set(0xFF44, 0);
  }
u8 MemoryMapper::select_background_tile(u8 x, u8 y) {
    return ram[0x1800 + y * 32 + x];
}

u8 MemoryMapper::select_tile_pixel(u8 tile_index, u8 x, u8 y) {
    if (x >= 8 || y >= 8) { log("error"); return 0; }
    u8 * tile_ptr = &vram[tile_index * 16 + 2 * y];
    u8 t1 = tile_ptr[0];
    u8 t2 = tile_ptr[1];
    x = 7 - x;
    u8 v1 = (t1 >> x) & 1;
    u8 v2 = (t2 >> x) & 1;
    return v1 * 2 + v2;
  }
void MemoryMapper::load_cart(u8 * cart, u32 len) {
  if (len > 0x8000) len = 0x8000;
  for(u32 i=0; i < len; i++) {
    rom[i] = cart[i];
  }
}
