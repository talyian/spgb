#pragma once
#include "../base.hpp"
#include "io_ports.hpp"
#include "../utils/audio_stream.hpp"

u16 get_pc();
u64 get_monotonic_timer();

struct Audio;

struct Square {
  // These 5 bytes correspond to the 5 registers the CPU controls the square
  // wave with. The FIELD macro will define getter/setters for logical fields
  // based on the data in these bytes.
  u8 data[5];

  Audio* parent = 0;

  #define FIELD(f, index, len, offset)                                 \
    u8 f() { return (data[index] >> offset) & ((1 << len) - 1); }       \
    void f(u8 v) {                                                      \
      u8 mask = ((1 << len) - 1) << offset;                             \
      data[index] = ((v << offset) & mask) | (data[index] & ~mask); }

  FIELD(sweep_period, 0, 3, 4);
  FIELD(sweep_negate, 0, 3, 3);
  FIELD(sweep_shift, 0, 3, 0);

  FIELD(duty,    1, 2, 6);
  FIELD(length_counter, 1, 6, 0);

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
  void set(u8 addr, u8 val);

  u32 volume_ticker = 0;

  u32 ticks_4hz = 0; // 4Mhz ticks up to 8192 = 512hz(2ms)
  u32 counter_512hz = 0;
  // each chunk is 1.953ms, which is 93.75 samples at 48k
  u32 sample_counter = 0;// 4Mhz ticks up to 80   = 50khz(20us)
  u64 monotonic_counter = 0;

  void tick(u32 dt); 
  void add_audio_sample();
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
    sq0.parent = this;
    sq1.parent = this;
  }

  LongSampleQueue channels[4];
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

  bool power() { return AU16_power & 0x80; }
  u8 read(u16 addr) {
    u8 v = _read(addr);
    return v;
  }
  u8 _read(u16 addr); 

  void write(u16 addr, u8 val);
  void tick(u32 dt);

  void add_audio_sample(LongSample s) {
    auto& q = channels[s.channel];
    q.add(s);
  }

  void render_out(f32 sample_rate, u32 num_channels, i32 frames, f32 * data) {
    int j = 0;
    f32 out = 0;
    for (i32 i = 0; i < frames; i++) {
      f32 sample = channels[0].sample();
      data[j++] = sample;
      data[j++] = sample;
      out += sample;
      channels[0].tick({ (i32)(4 * 1024 * 1024 / sample_rate) });
    }
    if (out > 0) {
      log("oooh ", out);
    }
  }
};
