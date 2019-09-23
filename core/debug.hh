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
  vector<uint16_t> once_breakpoints;
  vector<span> watches;
  Debugger(Registers &r, Memory &m, PPU &p);
  void Step();
};
