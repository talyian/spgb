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

int main(int argc, char ** argv) {
  emulator_t emu {};

  if (argc > 1) {
    FILE * f = fopen(argv[1], "r");
    if (!f) { fprintf(stderr, "%s: file not found\n", argv[1]); exit(19); }
    fseek(f,0,SEEK_END);
    size_t len = ftell(f);
    fseek(f,0,0);
    u8 * buf = new u8[len];
    fread(buf, 1, len, f);
    fclose(f);
    emu.load_cart(buf, len);
  } else {
    printf("no file specified\n");
    return 18;
  }
  // emu.set_breakpoint(0x40); // vblank interrupt
  // emu.set_breakpoint(0xFF80); // high memory DMA loading thunk
  // emu.debug.is_debugging = true;
  // emu.debug.set_breakpoint(0xC66F); // Blargg 07 issue debugging
  // emu.debug.set_breakpoint(0xC681); // Blargg 07 issue debugging

  // emu.debug.set_breakpoint(0xc2b4); // Blargg 02 main function "EI"
  // emu.debug.set_breakpoint(0xc316); // Blargg 02 "timer doesn't work"
  u8 last_serial_cursor = 0, first_serial = 1;
  char line[64] {0};
  while(true) {
    emu.debug.step();

    if (emu.debug.is_debugging) {
      log("ime", emu.cpu.IME, "interrupt", emu.mmu.get(0xFFFF), emu.mmu.get(0xFF0F));
      log("timer", emu.timer.Control, emu.timer.DIV, emu.timer.TIMA,
          (u32)emu.timer.counter_t,
          emu.timer.monoTIMA,
          (u32)emu.timer.monotonic_t);
      emu._runner.dump();
      emu.printer.pc = emu.decoder.pc; emu.printer.decode();
      printf("DEBUG %04x> ", emu.decoder.pc);

      fgets(line, 63, stdin);
      for(int i = 0; i < 63; i++)
        if (!line[i]) break;
        else if (line[i] == '\n') { line[i] = 0; break; }
      
      if (!strcmp(line, "") || !strcmp(line, "s")) {
        emu.debug.is_stepping = true;
        emu.debug.is_debugging = false;
      }
      else if (!strcmp(line, "n")) {
        log("scanning to", emu.printer.pc);
        emu.debug.run_to_target = emu.printer.pc;
        emu.debug.is_debugging = false;
      }
      else if (!strcmp(line, "c")) {
        emu.debug.is_debugging = false;
      }
      else if (!strcmp(line, "q")) {
        break;
      }
      else {
        continue;
      }
    }
    
    emu.single_step();

    auto &serial = emu.cpu.serial;
    // if (serial.pos >= last_serial_cursor && serial.out_buf[serial.pos] == '\n') {
    if (last_serial_cursor < serial.pos) {
      // printf("\x1b[1;31m");
      if (first_serial) { printf("Serial: ");      first_serial = 0; }
      // blargg tests
      for(; last_serial_cursor < serial.pos; last_serial_cursor++) {
        u8 c = serial.out_buf[last_serial_cursor];
        if (c >= 0x20 || c == '\n')
          putchar(c);
        else
          printf("\\x%02x", c);
        if (c == '\n') {
          first_serial = 1;
        }
      }
      // printf("\x1b[0m");
      serial.out_buf[255] = 0;
      if (strstr((const char *)serial.out_buf, "Passed") ||
          strstr((const char *)serial.out_buf, "33") ||          
          strstr((const char *)serial.out_buf, "Failed")) {
        exit(0);
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
