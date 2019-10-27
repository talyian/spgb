#include "../base.hpp"
#include "../utils/audio_stream.hpp"
#include "../gb/lib_gb.hpp"
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <stdio.h>
#include <math.h>

#include <deque>
u64 get_monotonic_timer();

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
  void loop(Emulator emu, double elapsed_ms);

  u32 last_processed_dt = 0;
  LongSampleQueue sample_queue[4];
  void add_sample(u32 dt, u16 freq, u8 volume, u8 channel) { 
    sample_queue[channel].add({ dt, freq, volume, channel });
  }
};

AudioPlayer::AudioPlayer() {
#define CHECKFAIL if (FAILED(hr)) { printf("[%d]audio error %ld\n", __LINE__, hr); exit(1); }
  // Get default device
  IMMDeviceEnumerator* devices;
  
  HRESULT hr = CoCreateInstance(
    __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, _uuidof(IMMDeviceEnumerator), (void**)&devices);
  CHECKFAIL;
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
}


void AudioPlayer::loop(Emulator emu, double elapsed_ms) {
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

  spgb_audio_sample(emu,
    mix_format.nSamplesPerSec,
    mix_format.nChannels,
    available_frames, data);

  hr = audio_renderer->ReleaseBuffer(available_frames, 0);
  CHECKFAIL;
}

AudioPlayer* player = 0;

int audio_init() {
  player = new AudioPlayer();
  return 0;
}

void audio_loop(Emulator emu, double elapsed_ms) {
  if (player) player->loop(emu, elapsed_ms);
}
