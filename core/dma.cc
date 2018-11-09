#include "memory.hh"
#include "registers.hh"

// Every tick, if the CPU wrote to 0xFF46
// we take that address and copy it to the OAM segment
void checkDMA(__attribute__((unused)) Registers &reg, Memory &mem) {
  u8 arg = mem[0xFF46];
  if ((arg != 0xFF)) {
    mem[0xFF46] = 0xFF;
    for(u16 offset = 0; offset < 0xA0; offset++) {
      mem[0xFE00 + offset] = mem[arg * 0x100 + offset];
    }
    // technically after this bit, the OAM isn't accessible for X cycles
  }
};
