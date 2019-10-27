#pragma once
#include "../base.hpp"
#include "io_ports.hpp"
#include "../utils/audio_stream.hpp"

extern "C" f64 ceil(f64);
u16 get_pc();
u64 get_monotonic_timer();

struct Audio;

struct Square {
  // These 5 bytes correspond to the 5 registers the CPU controls the square
  // wave with. The FIELD macro will define getter/setters for logical fields
  // based on the data in these bytes.
  u8 data[5];
  LongSampleQueue qq;

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
  u8   get(u8 addr) { return data[addr] | mask[addr]; }
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
  LongSampleQueue qq;
  u8 data[5];

  FIELD(dac_power, 0, 1, 7);
  FIELD(counter, 1, 8, 0);
  FIELD(vol_bits, 2, 2, 5);
  FIELD(freq_lo, 3, 8, 0);
  FIELD(trigger, 4, 1, 7);
  FIELD(enable_counter, 4, 1, 6);
  FIELD(freq_hi, 4, 3, 0);

  u16 freq = 0;
  
  u8 table[0x10];
  u8 table_pos = 0;

  u8 status = 0;
  u8 channel = 2;
  u8 mask[5] = { 0x7F, 0xFF, 0x9F, 0xFF, 0xBF };
  u8   get(u8 addr) { return data[addr] | mask[addr]; }
  void set(u8 addr, u8 value) {
    data[addr] = value;
    if (addr == 0) {
      status = value;
      return;
    }
    if (addr == 1) {
      
    }
    if (addr == 3) {
      freq = freq_lo() | 0x100 * freq_hi();
      add_audio_sample();
      return;
    }
    if (addr == 4) {
      freq = freq_lo() | 0x100 * freq_hi();
      if (trigger()) {
        if (counter() == 0) { enable_counter(1); }
        table_pos = 0;
      }
      add_audio_sample();
      return;
    }
  }

  u64 counter_4mhz = 0;
  u64 monotonic_counter = 0;
  void tick(u32 dt) {
    counter_4mhz += dt;
    monotonic_counter += dt;
    while (counter_4mhz > 8192) {
      counter_4mhz -= 8192;
      tick_512();
    }
  }

  u32 counter_512hz = 0;
  void tick_512() {
    if (!status) return;
    if (counter_512hz++ % 2 == 0) {
      if (enable_counter()) {
        counter(counter() - 1);
        if (counter() == 0) {
          enable_counter(0);
          status = 0;
          vol_bits(0);
          freq = 0;
          add_audio_sample();
        }
      }
    }
  }

  void add_audio_sample() {
    log((u32)monotonic_counter, "Wave sample", freq, vol_bits());
    u8 vol_map[4] = { 0, 0xF, 0x7, 0x3};
    u8 volume = vol_map[vol_bits() & 3];
    LongSample s {{(u32)monotonic_counter}, freq, volume};
    for(u8 i = 0; i < 16; i++) {
      s.table[2 * i] = table[i] >> 4;
      s.table[2 * i + 1] = table[i] & 0xF;
    }
    qq.add(s);
  }
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

  bool power() { return AU16_power & 0x80; }
  u8 read(u16 addr) {
    u8 v = _read(addr);
    return v;
  }
  u8 _read(u16 addr); 

  void write(u16 addr, u8 val);
  void tick(u32 dt);

  void render_out(f32 sample_rate, u32 num_channels, i32 frames, f32 * data) {
    // TODO: This is called in a pretty tight loop but doesnt fire until the
    // host audio api is ready to receive additional data. On my windows system
    // this takes happens about once every 2ms of real time.

    // However, the simulation has already run ahead of real-time, up to an entire frame, and is possibly just waiting on Vsync. This means we really only want to pull about 2ms of audio data out of the queue. The danger is that we pull less than 2ms out and the queue gets gradually longer and longer.

    // One graphics is 456 * 154 cycles and lasts 59.7275 seconds.
    // One audio frame at 48khz lasts 87.3813333 frames.
    // 803.65 audio frames per graphics frame.

    f32 frame_count = frames;
    f64 tick_count = 4 * 1024 * 1024 * frame_count / sample_rate;
    f64 tick_increment = ceil(4 * 1024 * 1024 / sample_rate);
    int j = 0;
    f32 out = 0;
    for (i32 i = 0; i < frames; i++) {
      i32 tick_update = (i * tick_increment - (i - 1) * tick_increment);
      sq0.qq.tick({tick_update});
      sq1.qq.tick({tick_update});
      wave.qq.tick({tick_update});
      f32 u = 0, v = 0, w = 0;
      u = sq0.qq.sample();
      v = sq1.qq.sample();
      w = wave.qq.sample();
      // DEBUG: just testing wave out
      // u = 0;
      // v = 0;

      data[j++] = u + v + w;
      data[j++] = u + v + w;
    }
  }
};
