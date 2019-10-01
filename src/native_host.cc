#include "base.hpp"
#include "wasm_host.hpp"
#include "emulator.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// imports
extern "C" {
  void _logf(double v) { printf("%f ", v); }
  void _logx8(u8 v) { printf("%02x ", v); }
  void _logx16(u16 v) { printf("%04x ", v); }
  void _logx32(u32 v) { printf("%04x ", v); }
  void _logs(const char * s, u32 len) { printf("%.*s ", len, s); }
  void _showlog() { printf("\n"); }
  void _stop() { exit(1); }
  void _logp(void * v) { printf("%p ", v); }
}

extern "C" void * get_emulator();
extern "C" void   step_frame(void * emulator);

int main() {
  emulator_t emu;
  // emu.set_breakpoint(0x40); // vblank interrupt
  // emu.set_breakpoint(0xFF80); // high memory DMA loading thunk
  emu.debug.is_debugging = true;
  while(true) {
    emu.step(42000);
    if (emu.debug.is_debugging) {
      printf("%04x >> ", emu.decoder.pc);
      char * line;
      size_t n = 0;
      int len = 0;
      len = getline(&line, &n, stdin);
      if (len > 0) line[len-1] = 0;
      else continue;
      if (!strncmp(line, "?", len)) {
        printf(R"XXX(
Debug commands:
 (q) - quit

 (s) - step
 (c) - run
 (r) - run to next return
 (n) - run to next line

 (bra) - add breakpoint
 (brc) - clear breakpoint
 (brl) - list breakpoints

 (r) - show registers
 (d) - disassemble at PC
 (d xxxx) - disassemble at instruction

)XXX"); }
      if (!strncmp(line, "q", len)) { exit(0); }
      if (!strncmp(line, "c", len)) { emu.debug.is_debugging = false; }
      if (len > 2 && line[0] == 'r' && line[1] == ' ') {
        u16 addr = 0;
        for(char * p = line + 2; *p; p++) {
          u8 d = (*p - '0') % 32;
          if (d >= 17) d -= 7;
          addr = addr * 0x10 + d;
        }
        emu.debug.break_temp = addr;
        emu.debug.is_debugging = false;
      }
      if (!strncmp(line, "", 1) || !strncmp(line, "s", len)) {
        emu.debug.is_stepping = true;
        emu.debug.is_debugging = false;
      }
    }
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
  return;
  if ((category & ~0xFF) == 0x100)
    _show_tile(memory,len);
  else if ((category & 0xFF)  == 0x200)
    _show_bg_map(memory,len);
  else
    ; // log("pushframe");
}
