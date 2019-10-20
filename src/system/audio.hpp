#pragma once
#include "../base.hpp"
#include "io_ports.hpp"

u16 get_pc();
struct Audio {
  u8 data[0x30];
  
  u8 mask[0x30] = {
    0x80, 0x3F, 0x00, 0xFF, 0xBF, 
    0xFF, 0x3F, 0x00, 0xFF, 0xBF,
    0x7F, 0xFF, 0x9F, 0xFF, 0xBF,
    0xFF, 0xFF, 0x00, 0x00, 0xBF,
    0x00, 0x00, 0x70,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // UNUSED
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // Wave RAM
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  };

  bool power() { return data[0x16] & 0x80; }
  
  u8 read(u16 addr) {
    // wave ram ignores power and mask
    _log(get_pc());
    if (addr >= 0x20) {
      log("AU W ", data[addr]);
      return data[addr];
    }
    // if off, return just the mask
    if (!power()) {
      log("AU 0 ", addr, mask[addr]);
      return mask[addr];
    }
    else {
      log("AU 1 ", addr, (u8)(data[addr] | mask[addr]));
      return data[addr] | mask[addr];
    }
  }

  void write(u16 addr, u8 val) {

    // log(get_pc(), "AU::write", addr, val);
    if (addr == 0x16) {
      if (val & 0x80) {
        data[addr] |= 0x80;
      } else {
        // data[addr] &= ~0x80;
        for(u8 i = 0; i< 0x20; i++) {
          data[i] = 0;
        }
      }
    }
    else if (!power()) { }
    else
      data[addr] = val;
  }

  void tick(u32 dt) {
    
  }
};
