#pragma once
#include <cstdint>
typedef uint8_t u8;
typedef uint16_t u16;

struct Registers {
  u8 A = 0, B = 0, C = 0, D = 0, E = 0, F = 0, H = 0, L = 0;
  u16 SP = 0, PC = 0, _PC = 0; // the previous PC
  // R16 is a struct that aliases access to two 8-bit registers;
  struct R16 {
    u8 &ll, &hh;
    R16(u8 &high, u8 &low) : ll(low), hh(high) { }
    operator u16 () const { return ll + 256 * hh; }
    void operator = (u16 v) { ll = v; hh = v >> 8; }
  } BC {B, C}, DE{D, E}, HL{H, L}, AF{A, F};

  template<int bit> void setBit(u8 v) { F &= ~(1 << bit); F |= (v & 1) << bit; }
  template<int bit> u8 getBit() { return 1 & (F >> bit); }
  u8 FC() { return getBit<4>(); }
  u8 FH() { return getBit<5>(); }
  u8 FO() { return getBit<6>(); }
  u8 FZ() { return getBit<7>(); }
  void setFC(bool v) { setBit<4>(v); }
  void setFH(bool v) { setBit<5>(v); }
  void setFO(bool v) { setBit<6>(v); }
  void setFZ(bool v) { setBit<7>(v); }
  void setF(bool z, bool n, bool h, bool c) { F = z << 7 | n << 6 | h << 6 | c << 4; }
  void dump();
};
