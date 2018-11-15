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
struct Memory {
  u8 bios[0x100];

  // Ram sections go here
  // struct VM {
  //   u8 ROM0[0x4000];
  //   u8 ROM1[0x4000]; // 0x4000
  //   struct {
  //     u8 Tiles1[0x800];
  //     u8 Tiles2[0x800];
  //     u8 Tiles0[0x800];
  //     u8 Map1[0x400];
  //     u8 Map2[0x400];
  //   }  __attribute__((packed)) VRAM;          // 0x8000
  //   u8 RAM1[0x2000]; // 0xA000
  //   u8 RAM0[0x3E00]; // 0xC000
  //   u8 OAM[0xA0];    // 0xFE00
  //   u8 unused1[0x60];
  //   IO8 IO[0x80];    // 0xFF00
  //   u8 HRAM[0x80];   // 0xFF80
  // } __attribute__((packed)) mem;
  u8 mem[0x10000];

  u8 &exit_bios = mem[0xFF50]; // If set, BIOS is disabled
  Sprite* oam = (Sprite*)&mem[0xFE00];
  void set(u8 addr, u8 value) { mem[addr] = value; }
  u8 get(u16 addr) {
    if (!exit_bios && addr < 0x100) { return bios[addr]; }
    return mem[addr];
  };
  u8 &operator [](u16 addr) {
    #ifdef PROTECT_IO_SLOTS
    if (((addr & 0xFF00) == 0xFF00) && (addr < 0xFF80)) {
      if (0xFF10 <= addr && addr < 0xFF40) {
        // sound registers
      } else
      switch(addr) {
      case 0xFF00: break; // Joypad
      case 0xFF01: // Serial control
      case 0xFF02: break;
      // case 0xFF04:
      // case 0xFF05:
      // case 0xFF06:
      // case 0xFF07 :  // timer
      case 0xFF0F: break; // Interrupts Flag
      // case 0xFF10: break; // Sound - TODO
      case 0xFF40: break; // LCDC Status
      case 0xFF41: break; // STAT
      case 0xFF42: break; // SCY
      case 0xFF43: break; // SCX
      case 0xFF44: break; // LY
      case 0xFF45: break; // LYC
      case 0xFF47: break; // TODO: BGP = Background Palette
      case 0xFF46: break; // DMA
      case 0xFF48: break; // TODO: OP0 = Object Palette 0
      case 0xFF49: break; // TODO: OP1 = Object Palette 1
      case 0xFF4A: break; // TODO: WY
      case 0xFF4B: break; // TODO: WX
      case 0xFF50: break; // disable ROM
      case 0xFFFF: break; // Interrupt Enable
      default:
        printf("[%04hx] unknown signal: %04hX\n", addr, addr);
        abort();
      }
    }
    #endif
    if (!exit_bios && addr < 0x100) { return bios[addr]; }
    return mem[addr];
  }

  Memory(
    FILE * bios_file,
    FILE * cartridge)
  {
    memset(&mem, 0, 0x10000);
    if (bios_file) {
      if (fread(bios, 1, 0x100, bios_file) != 0x100) {
        fprintf(stderr, "could not find bios\n");
        abort();
      }
    }

    if (cartridge) {
      fseek(cartridge, 0, SEEK_END);
      auto buflen = ftell(cartridge);
      fseek(cartridge, 0, 0);
      fread(&mem, 1, (buflen > 0x10000 ? 0x10000 : buflen), cartridge);
    }

    mem[0xFF46] = 0xFF;
  }
};
