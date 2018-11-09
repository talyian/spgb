#include "instructions.hh"
#include "memory.hh"
#include "ppu.hh"
#include "registers.hh"
#include "utils_vector.hh"

#include <csignal>
#include <cstdint>

void signal_handler(int signum);

struct Debugger {
  Registers &reg;
  Memory &mem;
  PPU &ppu;
  OpPrinter printer;
  OpParser<OpPrinter> pprinter;
  vector<uint16_t> breakpoints;
  vector<span> watches;
  Debugger(Registers &r, Memory &m, PPU &p)
      : reg(r), mem(m), ppu(p), pprinter(reg, mem, printer) {
    signal(SIGINT, signal_handler);
    // breakpoints.push_back(0x219); // bgbtest - start of sprite placement function
    // breakpoints.push_back(0x241);    // bgbtest - setting sprite.y to random value
    // breakpoints.push_back(0x31f);    // bgbtest - random value function start
    // watches.push_back({0xc000, 0xc002}); // first two star sprites
    // watches.push_back({0xc140, 0xc1a0}); // first two star sprites
    // breakpoints.push_back(0xFF80);
    // breakpoints.push_back(0x48); // LCDC Interrupt
  }
  void Step();
};

bool is_stepping = false;
void signal_handler(__attribute__((unused)) int signum) {
  if (is_stepping)
    exit(1);
  is_stepping = true;
}

void Debugger::Step() {
  if (!is_stepping) {
    if (breakpoints.find(reg._PC) != breakpoints.end())
      is_stepping = true;
  }

  while (is_stepping) {
    if (watches.size)
      printf("-[watched]------------------------------------\n");
    for(auto && span: watches) {
      for(auto p = span.start; p < span.end; p++) {
        auto c = (p - span.start) % 8;
        if (!c) printf("[%04hx] ", p);
        printf("%02hhx ", mem[p]);
        if (c == 7 || (p + 1 == span.end)) printf("\n");
      }

    }
    printf("-[registers]----------------------------------\n");
    reg.dump();
    u16 _pc = reg._PC;
    u16 pc = reg.PC;
    printf("pc: %x, pc: %x\n", _pc, pc);
    printer.pc = _pc;
    pprinter.Step();
    reg._PC = _pc;
    reg.PC = pc;

    char input_s[32];
    fgets(input_s, 32, stdin);
    size_t len = strcspn(input_s, "\n");
    input_s[len] = '\0';
    #define CMD(b) !strcmp(input_s, b)
    if (CMD("b")) ppu.SendBackground();
    else if (CMD("br")) {
      uint32_t addr;
      if (printf("br> "), !scanf("%x", &addr)) { continue; }
      if (addr > 0xFFFF) { continue; }
      breakpoints.push_back(addr);
    }
    else if (CMD("brl")) { for(auto &b: breakpoints) printf("break $%04hx\n", b); printf("\n"); }
    else if (CMD("brc")) { breakpoints.clear(); }
    else if (CMD("brd")) {
      uint32_t index;
      if (!scanf("%x", &index)) { continue; }
      if (index > breakpoints.size) { continue; }
      breakpoints[index] = breakpoints.pop_back();
    }
    else if (CMD("\x1b[15~") || CMD("c")) { is_stepping = false; return; }
    else if (CMD("f")) ppu.SendForeground();
    else if (CMD("l")) ppu.SendScreen();
    else if (CMD("m")) {
      int start, end;
      if (printf("start >> "), !scanf("%x", &start)) { continue; }
      if (printf("end >> "), !scanf("%x", &end)) { continue; }
      watches.push_back({(u16)start, (u16)end});
    }
    else if (CMD("q")) exit(0);
    else if (CMD("r")) { /* todo: scan until the next ret and run there */ }
    else if (CMD("s")) return;
    else if (CMD("t")) ppu.SendTiles();
    else { printf("Error parsing command [%s]\n", input_s); }
    #undef CMD
  }
};
