#include "base.hpp"
#include "wasm_host.hpp"
#include <stdio.h>
#include <stdlib.h>

// imports
extern "C" {
  void _logf(double v) { printf("%f ", v); }
  void _logx8(u8 v) { printf("%02x ", v); }
  void _logx16(u16 v) { printf("%04x ", v); }
  void _logx32(u32 v) { printf("%04x ", v); }
  void _logs(const char * s, u32 len) { printf("%.*s ", len, s); }
  void _showlog() { printf("\n"); }
  void _stop() { exit(1); }
}

extern "C" void * get_emulator();
extern "C" void   step_frame(void * emulator);

int main() {
  auto instance = get_emulator();
  while(true) {
    step_frame(instance);
  }
}

void _show_tile(u8 * memory, u32 len) {
  printf("Tile Map  ========================================\n");
  u8 buffer[16 * 8 * 8 * 8];
  u8 STRIDE = 8 * 8;
  u8 * tile_data = memory;
  for(u8 y = 0; y < 8; y++) {
    for(u8 x = 0; x < 16; x++) {
      u8 s = 0;
      for(u8 i = 0; i < 16; i++) s += tile_data[i];

      for(u8 j = 0; j < 8; j++) {
        for(u8 i = 0; i < 8; i ++) {
          buffer[STRIDE * (y * 8 + j) + x * 8 + i] = s;
        }
      }
      tile_data += 16;
    }
  }
  u8 * p = buffer;
  for(u8 y = 0; y < 2 * 8; y++) {
    for(u8 x = 0; x < 16 * 8; x++) {
      printf("%c", ' ' + (*p & 0x4f));
      p++;
    }
    printf("\n");
  }
}
void _show_bg_map(u8 * memory, u32 len) {
  printf("BG Map ========================================\n");
  u8 * p = memory;
  for(u8 y = 0; y < 32; y++){ 
    for(u8 x = 0; x < 32; x++) {
      printf("%c", ' ' + (*p++ % 32));
    }
    printf("\n");
  }
}

void _push_frame(u32 category, u8 * memory, u32 len) {
  if ((category & ~0xFF) == 0x100)
    _show_tile(memory,len);
  else if ((category & 0xFF)  == 0x200)
    _show_bg_map(memory,len);
  else
    ; // log("pushframe");
}
