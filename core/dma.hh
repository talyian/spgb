#include "memory.hh"
#include "registers.hh"

// Every tick, if the CPU wrote to 0xFF46
// we take that address and copy it to the OAM segment
void checkDMA(__attribute__((unused)) Registers &reg, Memory &mem);
