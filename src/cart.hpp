#pragma once

#include "base.hpp"
#include "str.hpp"
#include "wasm_host.hpp"

struct Cart {
  u8 * data;
  u32 len; // expected to be between 32K and 4M
  str name;
  enum { CGB, CGB_Only, DMG, SGB } console_type;
  enum { ROM = 0, MBC1 = 1, ERROR } mapper;
  
  u32 rom_size;
  u8 * ram;
  u32 ram_size;
  
  Cart(u8 * data, u32 len) : data(data), len(len) {
    if (data == 0) return;

    name = {data + 0x134, 0xB};
        
    if (data[0x143] == 0x80) console_type = CGB;
    else if (data[0x143] == 0xC0) console_type = CGB_Only;
    else if (data[0x146] == 0x3) console_type = SGB;
    else console_type = DMG;

    rom_size = (32 * 1024) << data[0x148];

    if (data[0x149])
      ram_size = 1 << (2 * data[0x149] - 1);
    else
      ram_size = 0;
    ram = new u8[ram_size];
    
    if (data[0x147] == 0) mapper = ROM;
    else if (data[0x147] == 1) mapper = MBC1;
    else mapper = ERROR;

    _log("cart"); _log(name); _log(len);
    _log("cart-type"); _log(data[0x147]);
    _log("rom-size"); _log(data[0x148]); _log(rom_size);
    _log("ram-size"); _log(data[0x149]); _log(ram_size);
    _showlog();

    if (mapper != ROM && mapper != MBC1) {
      log("Unsupported Cart Mapper", (u8)mapper);
      _stop();
    }
  }

  struct MBC1 {
    u8 enable_ram = 0;
    u8 rom_bank = 1;
    u8 ram_bank = 0;
    enum BankMode : u8 { ROM, RAM } bank_mode;
  } mbc1;
  
  void write(u16 addr, u8 val) {
    if (mapper == ROM) {
      if (addr >= 0xA000) ram[addr - 0xA000] = val;
      return;
    }
    if (mapper == MBC1) {
      if (addr < 0x2000) {
        // log("MBC1: enable ram", val);
        mbc1.enable_ram = val == 0xA;
        return;
      }
      else if (addr < 0x4000) {
        u8 bank = 0x1F & val;
        bank += !bank;
        mbc1.rom_bank = (mbc1.rom_bank & ~0x1F) | bank;
        // log("MBC1: rom bank switch", val, mbc1.rom_bank);
        return;
      }
      else if (addr < 0x6000) {
        // log("MBC1: ram bank switch", addr, val, mbc1.bank_mode == MBC1::ROM ? "rom" : "ram");
        val = 0x3 & val;
        if (mbc1.bank_mode == MBC1::ROM)
          mbc1.rom_bank = (mbc1.rom_bank & ~0x60) | (val << 5);
        else
          mbc1.ram_bank = val;
        return;
      }
      else if (addr < 0x8000) {
        // log("MBC1: mode switch", val);
        mbc1.bank_mode = val == 0 ? MBC1::ROM : MBC1::RAM;
        return;
      }
      else if (addr < 0xC000) {
        ram[addr - 0xA000 + 0x2000 * mbc1.ram_bank] = val;
        return;
      }
    }
    log("unsupported mapper write", addr, val);
    _stop();
  }

  u8 read(u16 addr) {
    if (mapper == ROM) {
      return addr < 0xA000 ? data[addr] : ram[addr - 0xA000];
    }
    if (mapper == MBC1) {
      // bank 0 ROM
      if (addr < 0x4000) return data[addr];
      // bank 1 ROM
      if (addr < 0x8000) return data[(addr - 0x4000) + mbc1.rom_bank * 0x4000];
      // RAM
      if (addr < 0xC000) return ram[addr - 0xA000 + 0x2000 * mbc1.ram_bank];
    }
    log("unsupported mapper read", addr);
    _stop();
    return 0;
  }
};
