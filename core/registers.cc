#include "registers.hh"
#include <cstdio>

void Registers::dump() {
  printf("     A:   %02hhx ", A);
  printf(" B:   %02hhx ", B);
  printf(" C:   %02hhx ", C);
  printf(" D:   %02hhx ", D);
  printf(" E:   %02hhx ", E);
  printf(" F:   %02hhx  \n", F);
  printf("     HL: %04hx ", (u16) HL);
  printf("SP: %04hx ", SP);
  printf("PC: %04hx\n", PC);
}
