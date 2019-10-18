#pragma once

#include "../base.hpp"
#include "../platform.hpp"

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

  // the `flag<offset> struct` allows us to access
  // a single bit inside of the F register like a boolean.
  // "fl.Z = 1; fl.C = 1;" is equivalent to "registers.F & 0b10010000";
  template<int offset> struct bit {
    u8 &r;
    bit(u8 & _register) : r(_register) { }
    void operator=(bool n) { r ^= (((r >> offset) & 1) ^ n) << offset; }
    operator bool() { return (r >> offset) & 1; }
  };
  struct { 
    bit<7> Z;
    bit<6> N;
    bit<5> H;
    bit<4> C;
  } flags { {registers.F}, {registers.F}, {registers.F}, {registers.F} };

  
  // Interrupt Master Enable
  bool IME = 0;

  // HALT - sleep until interrupt triggers, resume at instruction
  // after the HALT when the interrupt handler finishes
  u8 halted = 0;
  // STOP - stop the machine.
  u8 stopped = 0;

  void clear() {
    memset(&registers, 0, sizeof(registers));
    IME = halted = stopped = 0;
  }
};

namespace logs { void _log(CPU::Reg16 v); }
