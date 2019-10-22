#pragma once
#include "../base.hpp"
#include "io_ports.hpp"
#include "timer.hpp"
#include "cart.hpp"
#include "audio.hpp"

struct MemoryMapper {
  MemoryMapper(Cart &cart, Audio &audio, IoPorts &io) :
    cart(cart), audio(audio), io(io), BiosLock(io.data[0x50]) {
    set(0xFF42, 0);
    set(0xFF43, 0);
    set(0xFF44, 0);
  }
  Cart &cart;
  Audio &audio;
  u8 *bios_rom = 0;
  
  // TODO put VRAM and OAM in the PPU
  u8 VRAM[0x2000];    // at 0x8000
  u8 WRAM[8][0x1000]; // work ram at 0xC000 / banks at 0xD000
  // u8 OAM[0x100];      // OAM at 0xFE00
  u8 * OAM = new u8[0x100];
  IoPorts &io;        // IO registers at 0xFF00
  // u8 HRAM[0x80];      // HRAM at 0xFF80
  u8 * HRAM = new u8[0x80];
  u8 error;

  u8 &BiosLock;
  
  void load_cart(const Cart &cart)  {
    this->cart = cart;
  }

  u8 get(u16 addr) {
    switch(addr >> 12) {
    case 0x0: return !BiosLock && addr < 0x100 ? bios_rom[addr] : cart.read(addr);
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7: return cart.read(addr);
    case 0x8:
    case 0x9: return VRAM[addr & 0x1FFF];
    case 0xA:
    case 0xB: return cart.read(addr);
    case 0xC: return WRAM[0][addr & 0xFFF];
    case 0xD: return WRAM[1][addr & 0xFFF];
    case 0xE: return WRAM[0][addr & 0xFFF]; // echo0
    default:
      return 
        addr < 0xFE00 ? WRAM[1][addr & 0xFFF] : // echo
        addr < 0xFF00 ? OAM[addr & 0xFF] :
        addr < 0xFF10 ? io.data[addr & 0xFF] :
        // addr < 0xFF40 ? audio.read(addr - 0xFF10) :
        addr < 0xFF80 ? io.data[addr & 0xFF] :
        HRAM[addr - 0xFF80];
    }
  }

  u16 get16(u16 addr) { return get(addr) + 0x100 * get(addr + 1); }
  
  void set(u16 addr, u8 val) {
    if (addr < 0x100 && !BiosLock) bios_rom[addr] = val;
    else if (addr < 0x8000) cart.write(addr, val);
    else if (addr < 0xA000) VRAM[addr - 0x8000] = val;
    else if (addr < 0xC000) cart.write(addr, val);
    else if (addr < 0xD000) WRAM[0][addr - 0xC000] = val;
    else if (addr < 0xE000) WRAM[1][addr - 0xD000] = val; // TODO : CGB: banking?
    else if (addr < 0xF000) WRAM[0][addr - 0xE000] = val; // echo
    else if (addr < 0xFE00) WRAM[1][addr - 0xF000] = val; // echo
    else if (addr < 0xFF00) OAM[addr - 0xFE00] = val;
    else if (addr == 0xFF02)  {
      if (val & 0x80) {
        _serial_putc(io.data[0x01]);
        io.data[0x02] = val & ~0x80;
      }
    }
    else if (addr < 0xFF10) { io.data[addr - 0xFF00] = val; }
    // else if (addr < 0xFF40) { audio.write(addr - 0xFF10, val); }
    else if (addr == 0xFF50) { BiosLock = 1; }
    else if (addr < 0xFF80) { io.data[addr - 0xFF00] = val; }
    else HRAM[addr - 0xFF80] = val;
  }

  void clear() {
    // memset(ram + 0x7F00, 0, 0x80);
  }
};
