#pragma once
#include "../base.hpp"
#include "io_ports.hpp"

u16 get_pc();
u64 get_monotonic_timer();

struct AudioFrame {
  f32 freq = 0;
  f32 volume = 0;
};

extern "C" void write_audio_frame_out(f32 freq, f32 volume);
extern "C" void write_1024_frame(u8 channel, f32 (&buffer)[1024]);
struct Square {
  u8 data[5];
  #define FIELD(f, index, len, offset)                                 \
    u8 f() { return (data[index] >> offset) & ((1 << len) - 1); }       \
    void f(u8 v) {                                                      \
      u8 mask = ((1 << len) - 1) << offset;                             \
      data[index] = ((v << offset) & mask) | (data[index] & ~mask); }

  FIELD(sweep_period, 0, 3, 4);
  FIELD(sweep_negate, 0, 3, 3);
  FIELD(sweep_shift, 0, 3, 0);

  FIELD(duty,    1, 2, 6);
  FIELD(counter, 1, 6, 0);

  u8 volume = 0;
  u8 volume_add = 0;
  u8 volume_period = 0;
  FIELD(vol_start,    2, 4 ,4); // unreadable
  FIELD(vol_add, 2, 1, 3);
  FIELD(vol_period,    2, 3, 0);

  u16 freq = 0;
  FIELD(freq_lo, 3, 8, 0);         // unreadable

  FIELD(trigger,        4, 1, 7);
  FIELD(enable_counter, 4, 1, 6);
  FIELD(freq_hi,        4, 3, 0);

  u8 mask[5] { 0x80, 0x3F, 0x00, 0xFF, 0xBF };
  // TODO: this part is probably backwards: we want a bool value
  // that the audio device maps to the addresss bit instead of vice versa
  u8 status = 0;
  u8 channel = 0; // just for debugging
  u8   get(u8 addr) { return ((u8*)this)[addr] | mask[addr]; }
  void set(u8 addr, u8 val) {
    ((u8*)this)[addr] = val;
    // log((u32) get_monotonic_timer(), get_pc(), "AU", channel, addr, " <- ", val);
    if (addr == 1) {
      // log("  - new duty/counter", duty(), counter());
    }
    if (addr == 2) {
      volume = vol_start();
      volume_add = 2 * vol_add() - 1;
      volume_period = vol_period();
      // log("  - AU", channel, val, "v", vol_start(), "+", vol_add(), "p", vol_period());
    }
    if (addr == 3) {
      freq = (freq_hi() << 8) | freq_lo();
      f32 period = (2048 - freq) * 8;
      // log("  - AU", channel, "freq", freq, (i32)freq, period,
      //     6 * 1000 * 1000 / period);
    }
    if (addr == 4 && val & 0x80) {
      status = 1;
      freq = (freq_hi() << 8) | freq_lo();
      if (!counter()) counter(0x3F);
      // log("  - AU", channel, "Trigger", val, enable_counter(), counter());
      // log("  - AU", channel, "freq", (u16)((freq_hi() << 8) | freq_lo()));
      // TODO frequency counter is reloaded with period /??
      // TODO volume envelope timer is reloaded with period /??
      volume = vol_start();
      volume_ticker = 0;
    }
  }

  u32 volume_ticker = 0;

  void tick_512hz() {
    if (enable_counter()) {
      // handle length counter
      counter(counter() - 1);
      if (counter() == 0) {
        status = 0;
      }
    }

    if (volume_period && volume < 0x10) {
      if (++volume_ticker == 8 * volume_period) {
        volume += volume_add;
        // log("AU", channel, "volume", volume);
        // if (volume < 0x10) write_audio_frame_out(freq, volume / 15.0);
        volume_ticker = 0;
      }
    }
  }

  u32 chunk_counter = 0; // 4Mhz ticks up to 8192 = 512hz(2ms)
  // each chunk is 1.953ms, which is 93.75 samples at 48k
  u32 sample_counter = 0;// 4Mhz ticks up to 80   = 50khz(20us)
  u32 sample_position = 0;

  void tick(u32 dt) {
    // dt is a 4Mhz clock tick.
    if (!status) { return; }
    chunk_counter += dt;
    sample_counter += dt;
    // our internal cycle is 512HZ, so we scale 8000x from 4Mhz
    while(chunk_counter >= 8192) {
      chunk_counter -= 8192;
      tick_512hz();
    }
  }

  // sample is called at 48khz, which is about once every 90 frames
  //
  u16 incrementor = 0;
  f32 sample() {
    if (!status) return 0;
    u8 v = volume < 0x10 ? volume : 0;
    u16 f = freq;
    // the period for f=0x783 is about 125 * 4;
    // we want it to be about 440 = 44k / 440 = 100 cycles
    u16 frequency_timer_period = (2048 - f) / 4;

    // f32 out =  (float)v / 15 * (incrementor++ % frequency_timer_period) / frequency_timer_period;
    // TODO: read duty value
    // this is a 50% duty cycle
    f32 out =  (f32)v / 15 * (incrementor < frequency_timer_period / 2 ? 1 : 0);

    incrementor++;
    if (incrementor == frequency_timer_period) {
      incrementor = 0;
    }
    return out;
  };
};

struct Wave {
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
  u8 table[0x10];
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
  Wave wave;   // FF1A - FF1E
  Noise noise; // FF1F - FF23

  u8 mask[0x30] = {
    0x80, 0x3F, 0x00, 0xFF, 0xBF,
    0xFF, 0x3F, 0x00, 0xFF, 0xBF,
    0x7F, 0xFF, 0x9F, 0xFF, 0xBF,
    0xFF, 0xFF, 0x00, 0x00, 0xBF,
    0x00, 0x00, 0x70,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // UNUSED
  };

  u8 AU16_power = 0; // FF26

  const static u32 BUFFER_LEN = 1024;
  bool audio_buffer = 0;
  f32 audio_out_left[2][BUFFER_LEN];
  f32 audio_out_right[2][BUFFER_LEN];
  u32 audio_position_left = 0;
  u32 audio_position_right = 0;
  const f32 OUTPUT_RESOLUTION_HZ = 48000;
  u32 out_counter = 0;

  bool power() { return AU16_power & 0x80; }
  u8 read(u16 addr) {
    u8 v = _read(addr);
    return v;
  }
  u8 _read(u16 addr); 

  void write(u16 addr, u8 val);
  void tick(u32 dt);

};
