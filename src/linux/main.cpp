#include "../base.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../gb/lib_gb.hpp"

// imports
extern "C" {
  void spgb_logf(double v) { printf("%f ", v); }
  void spgb_logx8(u8 v) { printf("%02x ", v); }
  void spgb_logx16(u16 v) { printf("%04x ", v); }
  void spgb_logx32(u32 v) { printf("%04x ", v); }
  void spgb_logs(const char * s, u32 len) { printf("%.*s ", len, s); }
  void spgb_showlog() { printf("\n"); }
  void spgb_stop() { exit(1); }
  void spgb_logp(void * v) { printf("%p ", v); }
  void spgb_serial_putc(u8 v) {
    static u64 sequence = 0;
    static char line[20];
    static u32 p = 0;
    auto flush = [&]() { printf("<<< %.*s\n", p, line); p = 0; };
    if (v == '\n' || p == 20) { flush(); } else { line[p++] = v; }
    sequence = sequence * 0x100 + v;
    if ((sequence & 0xFFFFFFFFFFFF) == *(u64 *)"dessaP\0") {
      flush();
      exit(0);
    }
    if ((sequence & 0xFFFFFFFFFFFF) == *(u64 *)"deliaF\0") {
      flush();
      exit(-1);
    }
  }
}

extern "C" void write_1024_frame(u8, f32 (&)[1024]) {
  
}

int main(int argc, char ** argv) {
  Emulator emu = spgb_create_emulator();

  if (argc > 1) {
    FILE * f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "%s: file not found\n", argv[1]); exit(19); }
    fseek(f,0,SEEK_END);
    size_t len = ftell(f);
    fseek(f,0,0);
    u8 * buf = new u8[len];
    fread(buf, 1, len, f);
    fclose(f);
    spgb_load_cart(emu, buf, len);
  } else {
    printf("no file specified\n");
    return 18;
  }
  
  while(true) {
    spgb_step_instruction(emu);
  }
}

void spgb_push_frame(u32 , u8 * , u32 ) {
  return;
}
