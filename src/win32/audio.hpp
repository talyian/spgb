#include "../base.hpp"
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <stdio.h>
#include <math.h>
#define CHECKFAIL if (FAILED(hr)) { printf("failed line %d\n", __LINE__); goto LFAIL; };
// code mostly copied from https://docs.microsoft.com/en-us/windows/win32/coreaudio/rendering-a-stream

struct AudioPlayer {
  IMMDeviceEnumerator * devices;
  IMMDevice * device;
  IAudioClient * audio_client;
  IAudioRenderClient * audio_renderer;
  WAVEFORMATEX * mix_format;
  HRESULT hr;
  REFERENCE_TIME requested_duration = 10000000, duration = 0;
  u32 buffer_frame_count = 0, flags = 0, padding_frames = 0, available_frames = 0;
  u8 * data = 0;
  bool error = 0;
  double timer = 0;
  u32 ticks = 0, ticks2 = 0;
  AudioPlayer();
  ~AudioPlayer();
  void loop(double elapsed_ms);
};


AudioPlayer::AudioPlayer() {
    hr = CoCreateInstance(
      __uuidof(MMDeviceEnumerator),
      NULL,
      CLSCTX_ALL,
      _uuidof(IMMDeviceEnumerator),
      (void **)&devices);
    CHECKFAIL;

    hr = devices->GetDefaultAudioEndpoint(
      EDataFlow::eRender,
      ERole::eConsole,
      &device);
    CHECKFAIL;

    hr = device->Activate(
      __uuidof(IAudioClient),
      CLSCTX_ALL,
      nullptr,
      (void **)&audio_client);
    
    CHECKFAIL;

    hr = audio_client->GetMixFormat(&mix_format);
    CHECKFAIL;

    {
      printf("channels=%d   bits/sample=%d   bits/sample=%lu\n",
             mix_format->nChannels,
             mix_format->wBitsPerSample,
             mix_format->nAvgBytesPerSec * 8 / mix_format->nSamplesPerSec);
      switch(mix_format->wFormatTag) {
      case WAVE_FORMAT_IEEE_FLOAT: printf("floating wave\n"); break;
      case WAVE_FORMAT_PCM: printf("PCM wave \n"); break;
      case WAVE_FORMAT_EXTENSIBLE: {
        auto ex_mix_format = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(mix_format);
        if(ex_mix_format->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
          printf("Extensible-ieee float\n");
          goto DONE;
        }
        if(ex_mix_format->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
          printf("Extensible-PCM\n");
          goto DONE;
        }          
        else
          printf("Extensible wave\n");
        break;
      }
      default:
        printf("unknown wave format\n");
      }
    }
DONE:
   
    hr = audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, requested_duration, 0,
                                  mix_format, nullptr);
    CHECKFAIL;

    // Get the actual size of the allocated buffer.
    hr = audio_client->GetBufferSize(&buffer_frame_count);
    CHECKFAIL;

    hr = audio_client->GetService(__uuidof(IAudioRenderClient),
                                  (void **)&audio_renderer);
    CHECKFAIL;

    // Grab the entire buffer for the initial fill operation.
    hr = audio_renderer->GetBuffer(buffer_frame_count, &data);
    CHECKFAIL;

    // Load the initial data into the shared buffer.
    printf("bufferframecount: %d\n", buffer_frame_count);
    for(u32 i = 0; i < buffer_frame_count; i++) {
      data[i] = i;
    }

    hr = audio_renderer->ReleaseBuffer(buffer_frame_count, flags);
    CHECKFAIL;

    // Calculate the actual duration of the allocated buffer.
    duration = (double)requested_duration * buffer_frame_count / mix_format->nSamplesPerSec;
    printf("requested duration: %llu\n", requested_duration);
    printf("actual    duration: %llu\n", duration);
    
    hr = audio_client->Start(); // Start playing.
    CHECKFAIL;

    timer = 0;
    // Wait for last data in buffer to play before stopping.
    // Sleep((DWORD)(duration / (REF_HZ / 1000) / 2));

    // hr = audio_client->Stop(); // Stop playing.
    CHECKFAIL;
    return;
  LFAIL:
    error = true;
    return;
  }

void AudioPlayer::loop(double elapsed_ms) {
    u32 out_index = 0;
    f32 * sample_data = 0;
    static i64 t = 0; // monotonic ticks @ 48Hz

    if (error) return;
    if (flags == AUDCLNT_BUFFERFLAGS_SILENT) return;
    timer += elapsed_ms;
    ticks++;

    // Sleep for half the buffer duration.
    if (timer < duration / (requested_duration / 1000) / 2) return;

    // See how much buffer space is available.
    padding_frames = 0;
    hr = audio_client->GetCurrentPadding(&padding_frames);
    CHECKFAIL;
    
    available_frames = buffer_frame_count - padding_frames;
    if (!available_frames) return;

    if (ticks2++ % 10 == 0) {
     printf("tick %d\n", ticks);
    }
    //  printf("1 audio loop %f / %llu\n", timer, duration);
    //  printf("available frames: %d\n", available_frames);
    //  printf("padding frames: %d\n", padding_frames);
    //  printf("total frames: %d\n", buffer_frame_count);
    //  printf("mix samples per ms  %lu\n", mix_format->nSamplesPerSec / 1000 );
    //}

    // Grab all the available space in the shared buffer.
    hr = audio_renderer->GetBuffer(available_frames, &data);
    CHECKFAIL;
    
    sample_data = (f32 *)data;
    out_index = 0;

    for(u32 i = 0; i < available_frames; i++) {
      t++; // one tick is one frame is 48khz, divided by 1000 is 480hz.
      f64 sinewave = sin(6.28 * t / 200.0);
      for(int j = 0; j < mix_format->nChannels; j++) { 
        sample_data[out_index++] = sinewave;
      }
      if (out_index != i * 2 + 2) {
        printf("oooops\n");
        _stop();
      }
      if ( out_index > available_frames * 2) {
        printf("oops\n");
        _stop();
      }
    }
    // Get next 1/2-second of data from the audio source.
    // hr = pMySource->LoadData(numFramesAvailable, pData, &flags);
    CHECKFAIL;
    
    hr = audio_renderer->ReleaseBuffer(available_frames, flags);
    CHECKFAIL;
  LFAIL:
    return;
  }

AudioPlayer::~AudioPlayer() {
    hr = audio_client->Stop(); // Stop playing.
    CHECKFAIL;
  LFAIL:
    return;
}


AudioPlayer * player = 0;

int audio_init() {
  player = new AudioPlayer();
  return 0;
}

void audio_loop(double elapsed_ms) {
  if (player) player->loop(elapsed_ms);
}
