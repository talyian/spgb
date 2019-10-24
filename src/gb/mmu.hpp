#pragma once
#include "../base.hpp"
#include "../utils/log.hpp"
#include "lib_gb.hpp"
#include "io_ports.hpp"
#include "timer.hpp"
#include "cart.hpp"
#include "audio.hpp"
#include "graphics.hpp"

struct MemoryMapper {
  MemoryMapper(Cart &cart, PPU &ppu, Audio &audio, IoPorts &io) :
    cart(cart), ppu(ppu), audio(audio), io(io), BiosLock(io.data[0x50]) {
    set(0xFF42, 0);
    set(0xFF43, 0);
    set(0xFF44, 0);
  }
  Cart &cart;
  PPU &ppu;
  Audio &audio;
  u8 *bios_rom = 0;
  
  u8 wram_bank = 1;
  u8 WRAM[8][0x1000];   // work ram at 0xC000 / banks at 0xD000
  u8 * OAM = ppu.OAM;   // OAM at 0xFE00
  IoPorts &io;          // IO registers at 0xFF00
  u8 HRAM[0x80];     // HRAM at 0xFF80
  u8 error;

  u8 &BiosLock;
  u8 cgb_mode = true;
  void load_cart(const Cart &cart)  {
    // Code smell: does this code belong in MemoryManager?
    this->cart = cart;
    if (cart.console_type == CGB) {
      ppu.Cgb.enabled = true;
    } else {
      ppu.Cgb.enabled = false;
      ppu.Cgb.bg_palette.pixels[3] = {0,0,0};
      ppu.Cgb.bg_palette.pixels[2] = {10,11,16};
      ppu.Cgb.bg_palette.pixels[1] = {20,22,18};
      ppu.Cgb.bg_palette.pixels[0] = {31,30,28};
      ppu.Cgb.spr_palette.pixels[3] = {0,0,0};
      ppu.Cgb.spr_palette.pixels[2] = {10,11,16};
      ppu.Cgb.spr_palette.pixels[1] = {20,22,18};
      ppu.Cgb.spr_palette.pixels[0] = {31,30,28};
      memset(ppu.VRAM2, 0, 0x2000);
    }
    log("cgb", (u8)cart.console_type, (u8)ppu.Cgb.enabled);
  }

  u8 get(u16 addr) {
    switch(addr >> 12) {
    case 0x0:
      return BiosLock || (addr >= 0x100 && addr < 0x200) ? cart.read(addr) : bios_rom[addr];
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7: return cart.read(addr);
    case 0x8:
    case 0x9: return ppu.vram_bank ? ppu.VRAM2[addr & 0x1FFF] : ppu.VRAM[addr & 0x1FFF];
    case 0xA:
    case 0xB: return cart.read(addr);
    case 0xC: return WRAM[0][addr & 0xFFF];
    case 0xD: return WRAM[wram_bank][addr & 0xFFF];
    case 0xE: return WRAM[0][addr & 0xFFF]; // echo0
    default:
      return 
        addr < 0xFE00 ? WRAM[wram_bank][addr & 0xFFF] : // echo
        addr < 0xFF00 ? OAM[addr & 0xFF] :
        addr < 0xFF10 ? io.data[addr & 0xFF] :
        addr < 0xFF40 ? audio.read(addr - 0xFF10) :
        addr < 0xFF4C ? ppu.read(addr - 0xFF40) :
        addr == 0xFF4F ? ppu.vram_bank :
        addr == 0xFF68 ? ppu.Cgb.bg_palette.addr :
        addr == 0xFF69 ? ppu.Cgb.bg_palette.read() :
        addr == 0xFF6A ? ppu.Cgb.spr_palette.addr :
        addr == 0xFF6B ? ppu.Cgb.spr_palette.read() :
        addr < 0xFF80 ? io.data[addr & 0xFF] :
        HRAM[addr - 0xFF80];
    }
  }

  u16 get16(u16 addr) { return get(addr) * 0x100 + get(addr + 1); }
  
  void set(u16 addr, u8 val) {
    if (addr < 0x100 && !BiosLock) {
      // bios_rom[addr] = val;
      log("writing to bios");
    }
    else if (addr < 0x8000) cart.write(addr, val);
    else if (addr < 0xA000)
      if (ppu.vram_bank)
        ppu.VRAM2[addr & 0x1FFF] = val;
      else
        ppu.VRAM[addr &0x1FFF] = val;
    else if (addr < 0xC000) cart.write(addr, val);
    else if (addr < 0xD000) WRAM[0][addr - 0xC000] = val;
    else if (addr < 0xE000) WRAM[wram_bank][addr - 0xD000] = val;
    else if (addr < 0xF000) WRAM[0][addr - 0xE000] = val; // echo
    else if (addr < 0xFE00) WRAM[wram_bank][addr - 0xF000] = val; // echo
    else if (addr < 0xFF00) OAM[addr - 0xFE00] = val;
    else if (addr == 0xFF02)  {
      if (val & 0x80) {
        spgb_serial_putc(io.data[0x01]);
        io.data[0x02] = val & ~0x80;
      }
    }
    else if (addr < 0xFF10) { io.data[addr - 0xFF00] = val; }
    else if (addr < 0xFF40) { audio.write(addr - 0xFF10, val); }
    else if (addr < 0xFF4C) { ppu.write(addr - 0xFF40, val); }
    else if (addr < 0xFF80) {
      if (addr == 0xFF50) { BiosLock = 1; }
      else if (ppu.Cgb.enabled) {
        if (addr == 0xFF4D) { io.data[0x4D] |= (2  | (val & 1)); }
        else if (addr == 0xFF4F) { ppu.vram_bank = val & 1; }
        else if (addr == 0xFF55) {
          // HDMA
          u16 source = get(0xFF51) * 0x100 + (get(0xFF52) & 0xF0);
          u16 dest = (get(0xFF53) & 0x1F) * 0x100;
          dest += get(0xFF54) & 0xF0;
          dest |= 0x8000;
          i16 length = (val & 0x7F);
          u8 transfer_mode = val & 0x80;
          if (transfer_mode) { log("Error: unsupported Hblank DMA"); }
          while(length >=0) {
            for(u16 j = 0; j < 0x10; j++) {
              set(dest++, get(source++));
            }
            io.data[0x51] = source >> 8;
            io.data[0x52] = source;
            io.data[0x53] = dest >> 8;
            io.data[0x54] = dest;
            io.data[0x55] = length;
            length--;
          }
          // done
          io.data[0x55] = 0xFF;
        }
        else if (addr == 0xFF56) { /* TODO: CGB IR port */ }
        else if (addr == 0xFF68) { ppu.Cgb.bg_palette.addr = val; }
        else if (addr == 0xFF69) { ppu.Cgb.bg_palette.write(val); }
        else if (addr == 0xFF6A) { ppu.Cgb.spr_palette.addr = val; }
        else if (addr == 0xFF6B) { ppu.Cgb.spr_palette.write(val); }
        else if (addr == 0xFF70) {
          u8 v = val & 0x7; v += !v;
          wram_bank = io.data[0x70] = v;
        }
        // CGB: unknown IO port
        else { io.data[addr - 0xFF00] = val; }
      }
      // DMG: unknown IP port
      else { io.data[addr - 0xFF00] = val; }
    }
    else HRAM[addr - 0xFF80] = val;
  }

  void clear() {
    // memset(ram + 0x7F00, 0, 0x80);
  }
};
