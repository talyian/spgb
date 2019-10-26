#include "audio.hpp"
#include "../utils/log.hpp"

u8 Audio::_read(u16 addr) {
  // wave ram ignores power and mask
  if (addr >= 0x20) {
    // log("AU W ", data[addr]);
    return wave.table[addr - 0x20];
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

void Audio::write(u16 addr, u8 val) {
  u64 ts = get_monotonic_timer();
  // log((u32)ts, get_pc(), "AU::write", addr, val);
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
  else wave.table[addr - 0x20] = val;
}

void Audio::tick(u32 dt) {
  if (power()) { 
    sq0.tick(dt);
    sq1.tick(dt);
  }
}

void Square::add_audio_sample() {
  LongSample s = {
    (u32)monotonic_counter, freq, volume, channel
  };
  parent->add_audio_sample(s);
}
void Square::set(u8 addr, u8 val) {
  ((u8*)this)[addr] = val;
  if (addr == 1) {
  }
  if (addr == 2) {
    u8 old_volume = volume;
    volume = vol_start();
    volume_add = 2 * vol_add() - 1;
    volume_period = vol_period();
    volume_ticker = 0;
    if (volume != old_volume)
      add_audio_sample();
  }
  if (addr == 3) {
    u16 old_frequency = freq;
    freq = (freq_hi() << 8) | freq_lo();
    f32 period = (2048 - freq) * 8;
    if (freq != old_frequency) {
      add_audio_sample();
    }
  }
  if (addr == 4 && val & 0x80) {
    status = 1; // writing to  byte 5 turns on the wave unit
    u16 old_frequency = freq;
    freq = (freq_hi() << 8) | freq_lo();
    if (freq != old_frequency)
      add_audio_sample();
    if (!length_counter()) length_counter(0x3F);
    // TODO frequency counter is reloaded with period /??
    // TODO volume envelope timer is reloaded with period /??
    volume = vol_start();
    volume_ticker = 0;
  }
}

void Square::tick(u32 dt) {
    // dt is a 4Mhz clock tick.
    if (!status) { return; }
    ticks_4hz += dt;
    sample_counter += dt;
    monotonic_counter += dt;
    // our internal cycle is 512HZ, so we scale 8000x from 4Mhz
    while(ticks_4hz >= 8192) {
      ticks_4hz -= 8192;
      counter_512hz++;

      // decrement length at 256hz
      if (counter_512hz % 2 == 0) {
        if (enable_counter()) {
          // handle length counter
          length_counter(length_counter() - 1);
          if (length_counter() == 0) {
            status = 0;
            volume = 0;
            freq = 0;
            add_audio_sample();
          }
        }
      }

      // update volume envelope at 64hz
      if (counter_512hz % 8 == 7) {
        if (volume_period && volume < 0x10) {
          if (++volume_ticker == 8 * volume_period) {
            volume += volume_add;
            // log("AU", channel, "volume", volume);
            // if (volume < 0x10) write_audio_frame_out(freq, volume / 15.0);
            if (volume >= 0x10) {
              volume = 0;
              volume_period = 0;
            }
            add_audio_sample();
          }
        } 
      }
    }
  }
