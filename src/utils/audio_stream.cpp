#include "audio_stream.hpp"

void render_sample(LongSampleQueue qq, int available_frames, f32* data, int offset, int stride) {
  int index = 0;
  const f64 ticks_per_frame = 4 * 1024 * 1024 / (f64)48000;
  for (int i = 0; i < available_frames; i++) {
    i32 elapsed_ticks = (i32)(ticks_per_frame * i / available_frames) - (i32)(ticks_per_frame * (i - 1) / available_frames);
    qq.tick({ elapsed_ticks });
    data[index + offset] = qq.sample();
    index += stride;
  }
}

cycle_time operator+(cycle_time a, cycle_time_delta b) {
  return { a.value + b.value };
}

cycle_time_delta operator-(cycle_time a, cycle_time b) {
  return { static_cast<i32>(a.value - b.value) };
}
