#pragma once
#include "base.hpp"
#include "io_ports.hpp"
#include "timer.hpp"
#include "wasm_host.hpp"
#include "cart.hpp"

struct MemoryMapper {
  MemoryMapper(Cart &cart, IoPorts &io);
  Cart &cart;
  
  u8 *bios_rom = 0;

  u8 VRAM[0x2000];    // at 0x8000
  u8 WRAM[8][0x1000]; // work ram at 0xC000 / banks at 0xD000
  u8 OAM[0x100];      // OAM at 0xFE00
  IoPorts &io;        // IO registers at 0xFF00
  u8 HRAM[0x80];      // HRAM at 0xFF80
  u8 error;

  u8 &BiosLock;
  
  void load_cart(const Cart &cart);

  u8 get(u16 addr) {
    return 
      addr < 0x100 && !BiosLock ? bios_rom[addr] :
      addr < 0x8000 ? cart.read(addr) :
      addr < 0xA000 ? VRAM[addr - 0x8000] :
      addr < 0xC000 ? cart.read(addr) :
      addr < 0xD000 ? WRAM[0][addr - 0xC000] :
      addr < 0xE000 ? WRAM[1][addr - 0xD000] : // TODO : CGB: banking?
      addr < 0xF000 ? WRAM[0][addr - 0xE000] : // echo
      addr < 0xFE00 ? WRAM[1][addr - 0xF000] : // echo
      addr < 0xFF00 ? OAM[addr - 0xFE00] :
      addr < 0xFF80 ? io.data[addr - 0xFF00] :
      HRAM[addr - 0xFF80];
  }

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
    else if (addr < 0xFF80) io.data[addr - 0xFF00] = val;
    else HRAM[addr - 0xFF80] = val;
  }

  void clear() {
    // memset(ram + 0x7F00, 0, 0x80);
  }
};
