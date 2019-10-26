#pragma once
#include "../base.hpp"
#include "io_ports.hpp"
#include "../utils/audio_stream.hpp"

u16 get_pc();
u64 get_monotonic_timer();

LongSample state[2];
void push_audio_sample(LongSample s) {
  LongSample last = state[s.channel];
  if (last.frequency == s.frequency && last.volume == s.volume) return;
  spgb_audio_sample(s.tick_time.value, s.frequency, s.volume, s.channel);
}

struct Square {
  // These 5 bytes correspond to the 5 registers the CPU controls the square
  // wave with. The FIELD macro will define getter/setters for logical fields
  // based on the data in these bytes.
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
  void set(u8 addr, u8 val) {
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
        spgb_audio_sample(monotonic_counter, freq, volume, channel);
    }
    if (addr == 3) {
      u16 old_frequency = freq;
      freq = (freq_hi() << 8) | freq_lo();
      f32 period = (2048 - freq) * 8;
      if (freq != old_frequency) {
        spgb_audio_sample(monotonic_counter, freq, volume, channel);
      }
    }
    if (addr == 4 && val & 0x80) {
      status = 1; // writing to  byte 5 turns on the wave unit
      u16 old_frequency = freq; 
      freq = (freq_hi() << 8) | freq_lo();
      if (freq != old_frequency)
        spgb_audio_sample(monotonic_counter, freq, volume, channel);
      if (!length_counter()) length_counter(0x3F);
      // TODO frequency counter is reloaded with period /??
      // TODO volume envelope timer is reloaded with period /??
      volume = vol_start();
      volume_ticker = 0;
    }
  }


  u32 volume_ticker = 0;

  u32 ticks_4hz = 0; // 4Mhz ticks up to 8192 = 512hz(2ms)
  u32 counter_512hz = 0;
  // each chunk is 1.953ms, which is 93.75 samples at 48k
  u32 sample_counter = 0;// 4Mhz ticks up to 80   = 50khz(20us)
  u64 monotonic_counter = 0;

  void tick(u32 dt) {
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
            spgb_audio_sample(monotonic_counter, freq, volume, channel);
            volume_ticker = 0;
          }
        } 
      }
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
