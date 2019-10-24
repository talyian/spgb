#pragma once

#include "../base.hpp"
#include "../utils/log.hpp"

struct CPU {
  CPU() {
    memset(&registers, 0, sizeof(registers));
  }

  typedef u8 Reg8;
  struct Reg16 {
    u8 h, l;
    Reg16() = default;
    Reg16(u16 v) { h = v >> 8; l = v; }
    operator u16 () { return (u16)(h * 0x100 + l); }
    Reg16 operator-- ();
    Reg16 operator++ (int);
    Reg16 operator-- (int);
  };
  
  union Registers {
    struct { Reg8 B, C, D, E, A, F, H, L; };
    struct { Reg16 BC, DE, AF, HL, SP; };
  } registers;

#define FLAG(X, bit)                                \
  bool flags_##X() { return (registers.F >> bit) & 1; } \
  void flags_##X(bool v) { registers.F = (registers.F & ~(1 << bit)) | (v << bit); }
  FLAG(Z, 7)
  FLAG(N, 6)
  FLAG(H, 5)
  FLAG(C, 4)
#undef FLAG
  // Interrupt Master Enable
  bool IME = 0;

  // HALT - sleep until interrupt triggers, resume at instruction
  // after the HALT when the interrupt handler finishes
  u8 halted = 0;
  // STOP - stop the machine.
  u8 stopped = 0;

  void clear() {
    // memset(&registers, 0, sizeof(registers));
    IME = halted = stopped = 0;
  }
};

namespace logs { void _log(CPU::Reg16 v); }
