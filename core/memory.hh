#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>

typedef uint8_t u8;
typedef uint16_t u16;

struct IO8 { u8 val; };
struct Sprite { u8 py, px, tile, flags; };
struct span { u16 start, end; };

struct CartHeader {
  u8 entry_point[4];
  u8 nintendo_logo[0x30];
  u8 title[15];
  u8 is_color_gb; // 0x80 for color
  u8 licensee[2];
  u8 is_super_gb; // 0x3 for super, 0x0 for game boy
  u8 cartridge_type;
  u8 rom_size;
  u8 ram_size;
  u8 destination_code;
  u8 licensee_code;
  u8 mask_rom_version;
  u8 complement_check;
  u8 checksum[2];
};

struct Memory {
  u8 bios[0x100];
  u8 mem[0x10000];

  u8 &exit_bios = mem[0xFF50]; // If set, BIOS is disabled
  Sprite* oam = (Sprite*)&mem[0xFE00];
  CartHeader *cart_header = (CartHeader *)&mem[0x100];

  void set(u8 addr, u8 value) { mem[addr] = value; }
  u8 get(u16 addr) {
    if (!exit_bios && addr < 0x100) { return bios[addr]; }
    if (cart_header->cartridge_type == 0) return mem[addr];
    if (cart_header->cartridge_type == 1) return mem[addr];
    return 0;
  };
  u8 &operator [](u16 addr);
  Memory(FILE * bios_file, FILE * cartridge);
};

struct Ref {
  Memory &mem;
  u16 addr;
  Ref(Memory &mem, u16 addr) : mem(mem), addr(addr) { }
};

