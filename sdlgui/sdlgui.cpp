#include "sdlgui.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

using namespace std;

struct SharedData {
    char memory[0x10000];
    char display[0x10000]; // the full display area
    char screen[0x5a00]; // the visible screen
    char extra[0x1000]; // extra
} data0;

extern "C" void * get_emulator();
extern "C" void   step_frame(void * emulator);
  
int main() {
  auto emulator = get_emulator();

  SDL_Init(SDL_INIT_EVERYTHING);
  auto window = SDL_CreateWindow("gb", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * 2, 144 * 2, SDL_WINDOW_OPENGL);
  auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(renderer, 160, 144);

  auto frame_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 160, 144);
  for (int i = 0; i < 160 * 144; i++) {
    data0.screen[i] = rand();
  }

  while (true) {
    step_frame(emulator);
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT) break;
      if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) break;
    }
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(frame_buffer, NULL, &data0.screen, 160);
    SDL_RenderCopy(renderer, frame_buffer, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(1000 / 60);
  }
}

extern "C" {
  void _logf(double v) { }
  void _logx8(u8 v) { }
  void _logx16(u16 v) { }
  void _logx32(u32 v) { }
  void _logs(const char * s, u32 len) { }
  void _showlog() { }
  void _push_frame(u32 category, u8 * data, u32 len) {
    if (category == 0x300) {
      for(int i = 0; i < 160 * 144; i++)
        data0.screen[i] = data[i];
    }
  }
  void _stop() { }
}
