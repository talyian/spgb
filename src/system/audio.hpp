#pragma once
#include "../base.hpp"
#include "io_ports.hpp"

u16 get_pc();

struct Square {
  u8 unused_0:1;     // unreadable
  u8 sweep_period:3;
  u8 sweep_negate:1;
  u8 sweep_shift:3;
  
  u8 duty:2;
  u8 timeout:6;        // unreadable

  u8 volume:4;        // unreadable
  u8 add_mode:1;
  u8 period:3;
  
  u8 freq_lo;         // unreadable

  u8 trigger:1;       // unreadable
  u8 enable_timeout:1;
  u8 unused_1:3;      // unreadable
  u8 freq_hi:3;       // unreadable

  u8 mask[5] { 0x80, 0x3F, 0x00, 0xFF, 0xBF };
  // TODO: this part is probably backwards: we want a bool value
  // that the audio device maps to the addresss bit instead of vice versa
  u8 status = 0;
  u8 channel = 0; // just for debugging
  u8   get(u8 addr) { return ((u8*)this)[addr] | mask[addr]; }
  void set(u8 addr, u8 val) {
    ((u8*)this)[addr] = val;
    // log("AU", channel, addr, " <- ", val);
    if (addr == 1) {
      // log("new duty/timeout", duty, timeout);
    }
    if (addr == 4 && val & 0x80) {
      // log("AU", channel, "Trigger", val, trigger, enable_timeout);
      status = 1;
      if (!timeout) timeout = 0x3F;
      // frequency counter is reloaded with period /??
      // volume envelope timer is reloaded with period /??
    }
  }
  u32 elapsed_ticks = 0;
  void tick(u32 dt) {
    // dt is a 4Mhz clock tick.
    if (!status) { return; }
    elapsed_ticks += dt;

    // our internal cycle is 256kHZ, so we scale 16x from 4Mhz
    if (timeout)
      while(elapsed_ticks >= 16) {
        elapsed_ticks -= 16;
        // log("square", channel, "timeout tick", timeout);
        timeout--;
        // log("square", channel, "timeout tick", timeout);
      }
    
    if (timeout == 0) {
      status = 0;
      // log("square", channel, "timeout rundown");
    }
  }
};

struct Wave {
  Wave(u8 * table) : table(table) { }
  u8 * table;
  u8 dac_power:1;
  u8 unused_0:7;

  u8 counter;

  u8 unused_1:1;
  u8 volume:2;
  u8 unused_2:5;

  u8 freq_lo;
  
  u8 trigger:1;       // unreadable
  u8 enable_timeout:1;
  u8 unused_3:3;      // unreadable
  u8 freq_hi:3;       // unreadable
};

struct Noise {

};
struct Audio {
  Audio() {
    sq1.channel = 1;
    sq1.mask[0] = 0xFF;
  }
  
  u8 data[0x30];
  Square sq0;  // FF10 - FF14
  Square sq1;  // FF15 - FF19
  Wave wave{wave_table};   // FF1A - FF1E
  Noise noise; // FF1F - FF23

  u8 wave_table[0x10];
  // struct Control {
  //   u8 vin_out2:1;
  //   u8 vol_out2:3;
  //   u8 vin_out1:1;
  //   u8 vol_out1:3;
  //   u8 output_map;
  // } control;
  u8 mask[0x30] = {
    0x80, 0x3F, 0x00, 0xFF, 0xBF, 
    0xFF, 0x3F, 0x00, 0xFF, 0xBF,
    0x7F, 0xFF, 0x9F, 0xFF, 0xBF,
    0xFF, 0xFF, 0x00, 0x00, 0xBF,
    0x00, 0x00, 0x70,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // UNUSED
  };

  u8 AU16_power = 0; // FF26
  bool power() { return AU16_power & 0x80; }
  
  u8 read(u16 addr) {
    u8 v = _read(addr);
    // log(get_pc(), "APU read", addr, "=", v);
    return v;
  }
  u8 _read(u16 addr) {
    // wave ram ignores power and mask
    if (addr >= 0x20) {
      // log("AU W ", data[addr]);
      return wave_table[addr - 0x20];
    }
    // if off, return just the mask
    if (!power()) {
      // log("AU 0 ", addr, mask[addr]);
      return mask[addr];
    }
    if (addr == 0x16) {
      return AU16_power | mask[addr] | (sq0.status) | (sq1.status << 1);
    }
    if (addr < 5) { return sq0.get(addr); }
    if (addr < 10) { return sq1.get(addr - 5); }
    return data[addr] | mask[addr];
  }

  void write(u16 addr, u8 val) {
    // log(get_pc(), "AU::write", addr, val);
    if (addr == 0x16) {
      if (val & 0x80) {
        // TODO: currently only enable 2 square waves
        // since we're checking that they disable themselves
        // due to timeout length == 0
        AU16_power |= 0x80;
      } else {
        AU16_power &= ~0x80;
        memset(data, 0, 0x20);
        memset(&sq0, 0, 5);
        memset(&sq1, 0, 5);
      }
    }
    else if (!power()) { }
    else if (addr < 5) { sq0.set(addr, val); }
    else if (addr < 10) { sq1.set(addr - 5, val); }
    else if (addr < 0x20) data[addr] = val;
    else wave_table[addr - 0x20] = val;
  }

  void tick(u32 dt) {
    if (power()) { 
      sq0.tick(dt);
      sq1.tick(dt);
    }
  }
};
