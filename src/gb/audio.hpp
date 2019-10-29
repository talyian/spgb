#pragma once
#include "../base.hpp"
#include "io_ports.hpp"
#include "../utils/audio_stream.hpp"

extern "C" int rand();

extern "C" f64 ceil(f64);
u16 get_pc();
u64 get_monotonic_timer();

struct Audio;

struct Square {
  Square(u8* d) : data(d) {}
  // These 5 bytes correspond to the 5 registers the CPU controls the square
  // wave with. The FIELD macro will define getter/setters for logical fields
  // based on the data in these bytes.
  u8* data;
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
  u64 monotonic_counter = 0;

  void tick(u32 dt); 
  void tick_update_length();
  void tick_update_sweep();
  void tick_update_volume();

  void add_audio_sample();
};

struct Wave {
  Wave(u8* data) : data(data) { }
  LongSampleQueue qq;
  u8* data;

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

  u64 monotonic_counter = 0;
  void tick(u32 dt) {
    monotonic_counter += dt;
  }

  void tick_update_length() {
    if (!status) return;
    if (!enable_counter()) return;
    counter(counter() - 1);
    if (counter() == 0) {
      enable_counter(0);
      status = 0;
      vol_bits(0);
      freq = 0;
      add_audio_sample();
    }
  }

  void add_audio_sample() {
    // log((u32)monotonic_counter, "Wave sample", freq, vol_bits());
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
  Noise(u8* data) : data(data) { }
  // LongSampleQueue qq;
  u8* data;

  FIELD(counter, 1, 6, 0);
  u8 volume = 0;
  FIELD(vol_start,    2, 4 ,4); // unreadable
  FIELD(vol_add, 2, 1, 3);
  FIELD(vol_period,    2, 3, 0);
  FIELD(freq_shift, 3, 4, 4);
  FIELD(step, 3, 1, 3);
  FIELD(freq_d, 3, 3, 0);

  FIELD(trigger, 4, 1, 7);
  FIELD(enable_counter, 4, 1, 6);
  
  u8 table[0x10];
  u8 table_pos = 0;

  u8 status = 0;
  u8 channel = 3;
  u8 mask[5] = { 0xFF, 0xFF, 0x00, 0x00, 0xBF };
  u8   get(u8 addr) { return data[addr] | mask[addr]; }
  void set(u8 addr, u8 value) {
    data[addr] = value;
    log("set NOISE", addr, value);
    if (addr == 1) { add_audio_sample(); return; }
    if (addr == 2) { add_audio_sample(); return; }
    if (addr == 3) {
      // volume
      add_audio_sample();
      return;
    }
    if (addr == 4) {
      if (trigger()) {
        if (counter() == 0) { enable_counter(1); }
        table_pos = 0;
        status = 1;
        noise_val = 0xFF;
        volume = vol_start();
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
  }

  u32 counter_512hz = 0;
  u8 volume_ticker = 0;
  void tick_update_length() {
    if (!enable_counter()) return;
    counter(counter() - 1);
    if (counter() == 0) {
      enable_counter(0);
      status = 0;
      volume = 0;
      add_audio_sample();
    }
  }
  void tick_update_volume() {
    if (vol_period()) {
      if (++volume_ticker == vol_period()) {
        volume += vol_add() * 2 - 1;
        // log("AU", channel, "volume", volume);
        // if (volume < 0x10) write_audio_frame_out(freq, volume / 15.0);
        if (volume >= 0x10) {
          volume = 0;
          vol_period(0);
        }
        add_audio_sample();
        volume_ticker = 0;
      }
    }
  }

  LongSampleQueue qq;
  u16 noise_val = 0;
  void add_audio_sample() {
    u8 d = freq_d() << 1;
    d += !d;
    u16 period = (d << freq_shift()) - 1;
    u16 freq = 2048 - period;
    log((u32)monotonic_counter, "Noise sample", freq, volume);
    LongSample s {{(u32)monotonic_counter}, freq, (u8)(0xf * volume)};
    for(u8 i = 0; i < 32; i++) {
      s.table[i] = noise_val & 1;
      u16 carry = (noise_val >> 1 ) ^ noise_val;
      carry &= 1;
      noise_val = (noise_val >> 1) | (carry << 15);
      s.table[i] = (rand() % 8) * 0.2 ;
    }
    qq.add(s);
  }
};

struct Audio {
  Audio() {
    sq1.channel = 1;
    sq1.mask[0] = 0xFF;
  }

  u8 data[0x30];
  Square sq0{ data };  // FF10 - FF14
  Square sq1{ data + 5 };  // FF15 - FF19
  Wave wave{ data + 10 };   // FF1A - FF1E
  Noise noise{ data + 15 }; // FF1F - FF23
  u8 AU16_power = 0; // FF26

  bool power() { return AU16_power & 0x80; }
  u8 read(u16 addr) {
    u8 v = _read(addr);
    return v;
  }
  u8 _read(u16 addr); 

  void write(u16 addr, u8 val);

  u64 monotonic_t = 0;
  u32 ticks_to_next_frame = 0;
  u8 frame_counter = 0;
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
      noise.qq.tick({ tick_update });
      f32 u = 0, v = 0, w = 0, n = 0;
      u = sq0.qq.sample();
      v = sq1.qq.sample();
      w = wave.qq.sample();
      n = noise.qq.sample();
      data[j++] = u + v + w + n;
      data[j++] = u + v + w + n;
    }
  }
};

