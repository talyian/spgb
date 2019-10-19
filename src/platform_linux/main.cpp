#include "../base.hpp"
#include "../platform.hpp"
#include "../emulator.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" size_t sslen(const char * s) { return strlen(s); }

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
  void _serial_putc(u8 v) {
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

extern "C" void * get_emulator();
extern "C" void   step_frame(void * emulator);

int main(int argc, char ** argv) {
  emulator_t emu {};

  if (argc > 1) {
    FILE * f = fopen(argv[1], "rb");
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
  // emu.debug.set_breakpoint(0xC2E7); // Blargg combined "03 test strange"
  char line[64] {0};

  while(true) {
    emu.debug.step();

    if (emu.debug.state.type == Debugger::State::PAUSE) {
      log("ime", emu.cpu.IME, "interrupt", emu.mmu.get(0xFFFF), emu.mmu.get(0xFF0F));
      printf("Timer: CTL=%02x DIV=%02x TIMA=%02x, ticks=%08x\n",
             emu.timer.Control, emu.timer.DIV, emu.timer.TIMA, (u32)emu.timer.monotonic_t);
      log("timer", emu.timer.Control, emu.timer.DIV, emu.timer.TIMA,
          (u32)emu.timer.counter_t,
          emu.timer.monoTIMA,
          (u32)emu.timer.monotonic_t);

      // emu.small_dump();
      printf("DEBUG %04x> ", emu._dasher.PC);

      fgets(line, 63, stdin);
      for(int i = 0; i < 63; i++)
        if (!line[i]) break;
        else if (line[i] == '\n') { line[i] = 0; break; }
      
      if (!strcmp(line, "") || !strcmp(line, "s")) {
        emu.debug.state.type = Debugger::State::STEP;
      }
      else if (!strcmp(line, "n")) {
        // log("scanning to", emu._dasher.PC_next());
        // emu.debug.state.type = Debugger::State::RUN_TO;
        // emu.debug.state.addr = emu._dasher.PC_next());
        continue;
      }
      else if (!strcmp(line, "c")) {
        emu.debug.state.type = Debugger::State::RUN;
      }
      else if (!strcmp(line, "r")) {
        emu.debug.state.type = Debugger::State::RUN_TO_RET;
        emu.debug.state.call_depth = 1;
      }
      else if (!strcmp(line, "q")) {
        break;
      }
      else {
        continue;
      }
    }
    
    emu.single_step();
  }
}

void _push_frame(u32 , u8 * , u32 ) {
  return;
}
