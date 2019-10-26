#include "../base.hpp"
#include "../utils/audio_stream.hpp"
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <stdio.h>
#include <math.h>

#include <deque>
u64 get_monotonic_timer();
//
//struct SampleQueue {
//  std::deque<LongSample> samples;
//  void add(LongSample s) {
//    samples.push_back(s);
//    while (samples.size() > 4096) {
//      printf("%lld : %s     %d - %d \n", get_monotonic_timer(), "warning: sample overflow", samples.front().tick_time, last_sample_t);
//      samples.pop_front();
//    }
//  }
//
//  LongSample current_sample{ 0, 0, 0, 0 };
//  u32 last_sample_t = 0;
//  u64 t_samples = 0;
//
//  void render_current(i32 frames, f32* data) {
//    for (int i = 0; i < 2 * frames-2;) {
//      t_samples++;
//      auto period = (2048 - current_sample.frequency) / 4;
//      f32 sample = (t_samples % period) > (period / 2) ? 0.05 * current_sample.volume : 0;
//      // for now, send mono data to both left/right channels
//      data[i++] = sample;
//      data[i++] = sample;
//    }
//  }
//
//  void render(i32 frames, f32 * data) {
//    LongSample* next_sample = samples.empty() ? 0 : &samples.front();
//    // one frame is 48000 hz. one time_offset unit is 4 * 1024 * 1024 hz.
//    f32 frame_ratio = 262.144 / 3;
//    u32 elapsed_ticks = frames * frame_ratio;
//    u32 i = 0;
//    // simulate time passage and update current_sample 
//    // printf("AUD: f=%d v=%d\n", current_sample.frequency, current_sample.volume);
//    while (frames >= 0) {
//      if (!next_sample) {
//        render_current(frames, data); 
//        last_sample_t -= elapsed_ticks;
//        return;
//      }
//
//      auto delay = next_sample->tick_time - last_sample_t;
//      if (delay <= 0) {
//        // head of queue is in the past
//        current_sample = * next_sample;
//        samples.pop_front();
//        next_sample = samples.empty() ? 0 : &samples.front();
//        continue;
//      }
//      // ===== head of queue is in the far future
//      else if (delay > elapsed_ticks) {
//        render_current(frames, data);
//        last_sample_t += elapsed_ticks;
//        return;
//      }
//      // ===== head of queue is in the near future
//      else {
//        u32 remaining_frames = delay / frame_ratio;
//        if (remaining_frames > frames) {
//          printf("oops\n");
//          exit(345);
//        }
//        render_current(remaining_frames, data);
//        frames -= remaining_frames;
//        elapsed_ticks = frames * frame_ratio;
//        data += 2 * remaining_frames;
//        current_sample = *next_sample;
//        last_sample_t += delay;
//        samples.pop_front();
//        next_sample = samples.empty() ? 0 : &samples.front();
//        continue;
//      }
//    }
//  }
//};
//

// code mostly copied from https://docs.microsoft.com/en-us/windows/win32/coreaudio/rendering-a-stream
struct AudioPlayer {
  bool error = 1;
  IMMDevice* device;
  IAudioClient* audio_client;
  IAudioRenderClient* audio_renderer;

  u32 buffer_length = 0; // buffer length in frames

  double elapsed_s = 0;
  u32 elapsed_frames = 0;
  WAVEFORMATEX mix_format;
  AudioPlayer();
  void loop(double elapsed_ms);

  u32 last_processed_dt = 0;
  LongSampleQueue sample_queue[4];
  void add_sample(u32 dt, u16 freq, u8 volume, u8 channel) { 
    sample_queue[channel].add({ dt, freq, volume, channel });
  }
};

AudioPlayer* g_audio = 0;

extern "C" void spgb_audio_sample(u32 dt, u16 freq, u8 volume, u8 channel) {
  if (g_audio) g_audio->add_sample(dt, freq, volume, channel);
}

AudioPlayer::AudioPlayer() {
#define CHECKFAIL if (FAILED(hr)) { printf("[%d]audio error %ld\n", __LINE__, hr); exit(1); }
  // Get default device
  IMMDeviceEnumerator* devices;
  HRESULT hr = CoCreateInstance(
    __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, _uuidof(IMMDeviceEnumerator), (void**)&devices);
  if (FAILED(hr)) return;
  hr = devices->GetDefaultAudioEndpoint(
    EDataFlow::eRender,
    ERole::eConsole,
    &device);
  CHECKFAIL;

  // Get audio_client
  hr = device->Activate(
    __uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audio_client);
  CHECKFAIL;

  u32 samples_per_second = 48000;
  u32 buffer_length_req = 2 * 1024;
  WAVEFORMATEXTENSIBLE WaveFormat;
  auto _format = &WaveFormat.Format;
  _format = &WaveFormat.Format;
  _format->cbSize = sizeof(WaveFormat);
  _format->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
  _format->wBitsPerSample = 32;
  _format->nChannels = 2;
  _format->nSamplesPerSec = samples_per_second;
  _format->nBlockAlign = (WORD)(2 * 32 / 8);
  _format->nAvgBytesPerSec = _format->nSamplesPerSec * _format->nBlockAlign;
  WaveFormat.Samples.wValidBitsPerSample = 32;
  WaveFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
  WaveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
  mix_format = *_format;
  REFERENCE_TIME buffer_duration = 10000000ULL * buffer_length_req / samples_per_second;
  hr = audio_client->Initialize(
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_STREAMFLAGS_NOPERSIST,
    buffer_duration,
    0, _format, nullptr);
  CHECKFAIL;

  // Get the actual size of the allocated buffer.
  hr = audio_client->GetBufferSize(&buffer_length);
  CHECKFAIL;
  printf("WASAPI: requested %d-size buffer, got %d\n", buffer_length_req, buffer_length);
  printf("WASAPI: buffer duration: %f ms\n", 1000.0 * buffer_length / _format->nSamplesPerSec);
  printf("WASAPI: samples/sec: %lu\n", _format->nSamplesPerSec);

  hr = audio_client->GetService(__uuidof(IAudioRenderClient), (void**)&audio_renderer);
  CHECKFAIL;
  hr = audio_client->Start(); // Start playing.
  CHECKFAIL;
  error = false;
  g_audio = this;
}


void AudioPlayer::loop(double elapsed_ms) {
  // printf("WASAPI: step: %d\n", 0);
  if (error) return;
  // printf("WASAPI: step: %d\n", 1);
  // if (volume_target <= 0) return;

  this->elapsed_s += elapsed_ms / 1000;
  // this->elapsed_frames += elapsed_ms * mix_format->nSamplesPerSec / 1000;
  u32 padding_frames;
  HRESULT hr = audio_client->GetCurrentPadding(&padding_frames);
  CHECKFAIL;

  u32 available_frames = buffer_length - padding_frames;
  if (!available_frames) return;
  f32* data;
  hr = audio_renderer->GetBuffer(available_frames, (u8**)&data);
  CHECKFAIL;

  render_sample(sample_queue[0], available_frames, data, 0, 2);
  render_sample(sample_queue[1], available_frames, data, 1, 2);

  hr = audio_renderer->ReleaseBuffer(available_frames, 0);
  CHECKFAIL;
}

AudioPlayer* player = 0;

int audio_init() {
  player = new AudioPlayer();
  return 0;
}

void audio_loop(double elapsed_ms) {
  if (player) player->loop(elapsed_ms);
}
