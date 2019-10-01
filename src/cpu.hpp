#include "base.hpp"
#include "wasm_host.hpp"

// Registers
typedef u8 reg8;
struct reg16 {
  u8 h, l;
  reg16() = default;
  reg16(u16 v) { h = v >> 8; l = v; }
  operator u16 () { return (u16)(h * 0x100 + l); }
  reg16 operator-- () {
    u16 v = (u16) *this;
    reg16 r = v - (u16)1;
    h = r.h;
    l = r.l;
    return *this;
  }
  reg16 operator++ (int) {
    u16 u = (u16) *this;
    reg16 r = u + (u16)1;
    h = r.h;
    l = r.l;
    return u;
  }
};

template<int offset>
struct bit {
  reg8 &r;
  bit(reg8 & _register) : r(_register) { }
  void operator=(bool n) {
    r ^= (((r >> offset) & 1) ^ n) << offset;
  }
  operator bool() { return (r >> offset) & 1; }
};

namespace logs {
void _log(reg16 v);
}

union Registers {
  struct { reg8 B, C, D, E, A, F, H, L; };
  struct { reg16 BC, DE, AF, HL, SP; };
};

struct Flags { 
  bit<7> Z;
  bit<6> N;
  bit<5> H;
  bit<4> C;
};

struct CPU {
  CPU() {
    memset(&registers, 0, sizeof(registers));
  }
  Registers registers;

  // the `flag<offset> struct` allows us to access
  // a single bit inside of the F register like a boolean.
  // "fl.Z = 1; fl.C = 1;" is equivalent to "registers.F & 0b10010000";
  Flags flags { {registers.F}, {registers.F}, {registers.F}, {registers.F} };

  // Interrupt Master Enable
  bool IME = 0;

  // HALT - sleep until interrupt triggers, resume at instruction
  // after the HALT when the interrupt handler finishes
  u8 halted = 0;
  // STOP - stop the machine.
  u8 stopped = 0;
};
