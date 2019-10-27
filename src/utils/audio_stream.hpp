#pragma once
#include "../base.hpp"
#include "../utils/log.hpp"

// We need to re-sample audio at a particular output frequency (48k by default).
// Counter for 4Ghz clock cycle ticks.
struct cycle_time { u32 value; };
struct cycle_time_delta { i32 value; };
cycle_time operator+(cycle_time a, cycle_time_delta b); 
cycle_time_delta operator-(cycle_time a, cycle_time b);
// Counter for 48Kz audio frame ticks.
struct frame_time { i32 value; };

struct LongSample {
  cycle_time tick_time;
  u16 frequency;
  u8 volume;
  u8 table[32];
  u8 channel;
};

// A ringbuffer of LongSample Values
struct LongSampleQueue {
  const static int QUEUESIZE = 1024;
  LongSample current{ 0, 0, 0, 0 };
  u32 read_pos = 0;
  u32 write_pos = 0;
  LongSample data[QUEUESIZE];

  // Enqueue a new audio sample
  void add(LongSample s) {
    auto write_pos_next = (write_pos + 1) % QUEUESIZE;
    if (write_pos_next == read_pos) {
      // overflow, we've wrapped around the entire buffer and are about to overwrite the read position. 
      // Catch read pointer up, this will cause some audio skip
      current = data[read_pos];
      read_pos = (read_pos + 1) % QUEUESIZE;
      // log("warning: audio buffer overrun");
    }
    data[write_pos] = s;
    write_pos = write_pos_next;
  }

  // Simulate the passage of cycle_time, update the current sample
  void tick(cycle_time_delta ticks) {
    monotonic_ticks += ticks.value;
    current.tick_time = current.tick_time + ticks;
    while (true) {
      if (read_pos == write_pos) 
        return; // queue is empty
      const LongSample &head = data[read_pos];
      auto elapsed = head.tick_time - current.tick_time;
      if (elapsed.value <= 0) {
        current = head;
        read_pos = (read_pos + 1) % QUEUESIZE;
      }
      else
        return;
    }
  }

  u64 monotonic_ticks = 0;
  f32 sample() {
    // return (monotonic_ticks / 10000 % 2 == 0);
    if (!current.frequency) return 0;
    if (!current.volume) return 0;
    u16 period = (2048 - current.frequency) * 2;
    u16 index = monotonic_ticks / period % 32;
    u8 wave = current.table[index];
    wave = (index / 16) % 2 == 0;
    return 0.05f * current.volume * wave;
  }
};

