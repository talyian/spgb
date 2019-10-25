#pragma once

#include "../base.hpp"
#include "../utils/log.hpp"
#include "../utils/str.hpp"
#include "lib_gb.hpp"

enum ConsoleType : u8 { CGB, CGB_Only, DMG, SGB };
enum class BankMode : u8 { ROM, RAM };

struct Mapper {
  enum { NONE, MBC1, MBC3, MBC5, ERROR } type;
  Mapper() : mbc1({}) { }
  union {
    struct MBC1Data {
      u8 enable_ram = 0; 
      u8 rom_bank = 1;
      u8 ram_bank = 0;
      // The Mapper:MBC1 mapper only reads 5 bits of the rom bank value. It
      // can interpret RAM bank writes as an additional top 2 bits of
      // the rom bank, if it's in ROM banking mode.
      BankMode bank_mode = BankMode::ROM;
    } mbc1;
    struct MBC3Data {
      u8 enable_ram = 0;
      u8 rom_bank = 1;
      u8 ram_bank = 0;

      u8 rtc_index = -1;
      u8 latch_rtc_val = 0xFF;
      bool latch_rtc;
      u8 rtc_seconds;
      u8 rtc_minutes;
      u8 rtc_hours;
      u8 rtc_daysLow;
      u8 rtc_control;
      u32 rtc_last_updated = 0;
      void update_rtc() {
        u32 ts = spgb_get_timestamp();
        u32 last_value = rtc_control & 1;
        last_value = last_value * 256 + rtc_daysLow;
        last_value = last_value * 24 + rtc_hours;
        last_value = last_value * 60 + rtc_minutes;
        last_value = last_value * 60 + rtc_seconds;
        u32 new_value = ts - rtc_last_updated + last_value;
        rtc_seconds = new_value % 60;
        rtc_minutes = (new_value /= 60) % 60;
        rtc_hours = (new_value /= 60) % 24;
        rtc_daysLow = (new_value /= 24) % 256;
        rtc_control &= ~0x81;
        rtc_control |= ((new_value /= 256) & 1);    // bit 9, days
        rtc_control |= ((new_value /= 2) & 1) << 7; // carry bit, days
        // TODO enable/disable RTC
      }
    } mbc3;
    struct MBC5Data {
      u8 enable_ram = 0;
      u8 rom_bank = 1;
      u8 ram_bank = 0;
      u8 rtc_index = -1;
    } mbc5;
  };
};

struct Cart {
  u8 *data;
  u32 len; // expected to be between 32K and 4M
  str name;

  u32 rom_size;
  u8 *ram;
  u32 ram_size;
  ConsoleType console_type;
  Mapper mapper;

  Cart(u8 *data, u32 len) : data(data), len(len) {
    if (data == 0)
      return;

    name = {data + 0x134, 0xB};

    if (data[0x143] >= 0x80)
      console_type = CGB;
    else
      console_type = DMG;

    rom_size = (32 * 1024) << data[0x148];

    if (data[0x149])
      ram_size = 1024 << (2 * data[0x149] - 1);
    else
      ram_size = 0;
    ram = new u8[ram_size];

    if (data[0x147] == 0x00)
      mapper.type = Mapper::NONE;
    else if (data[0x147] <= 0x03)
      mapper.type = Mapper::MBC1;
    else if (0x0F <= data[0x147] && data[0x147] <= 0x13)
      mapper.type = Mapper::MBC3;
    else if (0x19 <= data[0x147] && data[0x147] <= 0x1E)
      mapper.type = Mapper::MBC5;
    else
      mapper.type = Mapper::ERROR;

    log("cart", name, len,
        "mapper-type", data[0x147],
        "rom-size", data[0x148], rom_size,
        "ram-size", data[0x149], ram_size);
    if (len < rom_size) { spgb_stop(); }
    if (mapper.type == Mapper::ERROR) {
      log("Unsupported Mapper", data[0x147]);
      spgb_stop();
    }
  }

  void write(u16 addr, u8 val) {
    if (mapper.type == Mapper::NONE) {
      if (addr >= 0xA000)
        ram[addr - 0xA000] = val;
      return;
    }
    if (mapper.type == Mapper::MBC1) {
      auto &mbc = mapper.mbc1;
      if (addr < 0x2000) {
        // log("MBC1: enable ram", val);
        mbc.enable_ram = val == 0xA;
      } else if (addr < 0x4000) {
        u8 bank = 0x1F & val;
        bank += !bank;
        mbc.rom_bank = (mbc.rom_bank & ~0x1F) | bank;
        // log("MBC1: rom bank switch", val, mbc1.rom_bank);
        return;
      } else if (addr < 0x6000) {
        // log("MBC1: ram bank switch", addr, val, mbc1.bank_mode == BankMode::ROM ? "rom" : "ram");
        val = 0x3 & val;
        if (mbc.bank_mode == BankMode::ROM)
          mbc.rom_bank = (mbc.rom_bank & ~0x60) | (val << 5);
        else
          mbc.ram_bank = val;
        return;
      } else if (addr < 0x8000) {
        // log("MBC1: mode switch", val ? "RAM" : "ROM", val);
        mbc.bank_mode = val == 0 ? BankMode::ROM : BankMode::RAM;
        return;
      } else if (addr < 0xC000) {
        ram[addr - 0xA000 + 0x2000 * mbc.ram_bank] = val;
        return;
      }
    }
    if (mapper.type == Mapper::MBC3) {
      auto &mbc = mapper.mbc3;
      if (addr < 0x2000) {
        mbc.enable_ram = val == 0xA; return;
      }
      if (addr < 0x4000) {
        if (val < 0x80) {
          mbc.rom_bank = val + !val;
          return;
        }
      }
      if (addr < 0x6000) {
        if (val < 0x4) {
          mbc.ram_bank = val;
          mbc.rtc_index = 0xFF;
          return;
        }
        if (0x8 < val && val < 0xC) {
          mbc.rtc_index = val;
          mbc.ram_bank = 0xFF;
          return;
        }
      }
      if (addr < 0x8000) {
        if (mbc.latch_rtc_val == 0x00 && val == 0x01)
          mbc.latch_rtc ^= 1;
        mbc.latch_rtc_val = val;
        return;
      }
      if (addr < 0xC000) {
        if (mbc.ram_bank < 4) {
          ram[addr - 0xA000 + 0x2000 * mbc.ram_bank] = val;
          return;
          
        }
        if (mbc.rtc_index < 0xFF) {
          switch(mbc.rtc_index) {
          case 0x08:
            mbc.rtc_seconds = val % 60;
            mbc.rtc_last_updated = spgb_get_timestamp();
            return;
          case 0x09:
            mbc.rtc_minutes = val % 60;
            mbc.rtc_last_updated = spgb_get_timestamp();
            return;
          case 0x0A:
            mbc.rtc_hours = val % 24;
            mbc.rtc_last_updated = spgb_get_timestamp();
            return;
          case 0x0B:
            mbc.rtc_daysLow = val;
            mbc.rtc_last_updated = spgb_get_timestamp();
            return;
          case 0x0C:
            mbc.rtc_control = val;
            mbc.rtc_last_updated = spgb_get_timestamp();
            return;
          default: log("Writing RTC register", mbc.rtc_index);
          }
        }
      }
    }
    if (mapper.type == Mapper::MBC5) {
      auto &mbc = mapper.mbc5;
      if (addr < 0x2000) { mbc.enable_ram = val == 0xA; return; }
      else if (addr < 0x3000) {
        mbc.rom_bank = (mbc.rom_bank & 0x100) | val;
        return; }
      else if (addr < 0x4000) {
        mbc.rom_bank = (mbc.rom_bank & 0x0FF) | (val ? 0x100 : 0);
        return;
      }
      else if (addr < 0x6000) {
        mbc.ram_bank = val & 0xF;
        return;
      }
      else if (addr < 0x8000) { }
      else if (addr < 0xA000) { }
      else if (addr < 0xC000) {
        ram[addr - 0xA000 + 0x2000 * mbc.ram_bank] = val;
        return;
      }
    }
    log("unsupported mapper write", addr, val);
    spgb_stop();
  }

  u8 read(u16 addr) {
    if (mapper.type == Mapper::NONE) {
      return addr < 0xA000 ? data[addr] : ram[addr - 0xA000];
    }
    if (mapper.type == Mapper::MBC1) {
      auto &mbc = mapper.mbc1;
      // bank 0 ROM
      if (addr < 0x4000)
        return data[addr];
      // bank 1 ROM
      if (addr < 0x8000) {
        u32 index = (addr - 0x4000) + mbc.rom_bank * 0x4000;
        // log("reading", mbc1.rom_bank, ":", addr, "::", index);
        return data[index];
      }
      // RAM
      if (addr < 0xC000)
        return ram[addr - 0xA000 + 0x2000 * mbc.ram_bank];
    }
    if (mapper.type == Mapper::MBC3) {
      auto &mbc = mapper.mbc3;
      // bank 0 ROM
      if (addr < 0x4000)
        return data[addr];
      // bank 1 ROM
      if (addr < 0x8000)
        return data[(addr - 0x4000) + mbc.rom_bank * 0x4000];
      if (addr < 0xC000) {
        if (mbc.ram_bank < 0xFF) {
          return ram[addr - 0xA000 + 0x2000 * mbc.ram_bank];
        } else {
          mbc.update_rtc();
          switch(mbc.rtc_index) {
          case 0x08: return mbc.rtc_seconds;
          case 0x09: return mbc.rtc_minutes;
          case 0x0A: return mbc.rtc_hours;
          case 0x0B: return mbc.rtc_daysLow;
          case 0x0C: return mbc.rtc_control;
          default: log("Reading RTC register", mbc.rtc_index);
          }
        }
      }
    }
    if (mapper.type == Mapper::MBC5) {
      auto mbc = mapper.mbc5;
      if (addr < 0x4000) // ROM 0
        return data[addr];
      if (addr < 0x8000) // ROM Bank
        return data[(addr - 0x4000) + mbc.rom_bank * 0x4000];
      if (addr < 0xC000) 
        return ram[addr - 0xA000 + 0x2000 * mbc.ram_bank];
    }
    log("unsupported mapper read", addr);
    spgb_stop();
    return 0;
  }
};
