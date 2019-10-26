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
    out_counter += dt;
    //// Once every (4M / 48000) ticks, we output one frame.
    //if (out_counter >= audio_position_left * 4 * 1024 * 1024 / OUTPUT_RESOLUTION_HZ / 1.3) {
    //  f32 sample = sq0.sample() + sq1.sample();
    //  audio_out_left[audio_buffer][audio_position_left++] = sample;
    //  audio_out_right[audio_buffer][audio_position_right++] = sample;
    //  if (audio_position_left == BUFFER_LEN) {
    //    out_counter = 0;
    //    audio_position_left = 0;
    //    audio_position_right = 0;
    //    write_1024_frame(0, audio_out_left[audio_buffer]);
    //    write_1024_frame(1, audio_out_right[audio_buffer]);
    //    audio_buffer = !audio_buffer;
    //  }
    //}
  }
}
