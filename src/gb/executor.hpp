#pragma once

#include "../base.hpp"
#include "cpu.hpp"
#include "mmu.hpp"

// Decodes an instruction and runs it.
struct Executor {
  Executor(CPU & cpu, MemoryMapper & mmu) : cpu(cpu), mmu(mmu) { }

  CPU & cpu;
  MemoryMapper & mmu;
  bool error = 0;
  u16 PC = 0;
  u16 PC_start = 0;
  u16 PC_next = 0;
  i32 cycles = 0;

  // basic operations
  u8 mmu_get(u16 addr);
  void mmu_set(u16 addr, u8 val);
  u16 _pop();
  void _push(u16 value);
  u8 _read_u8();
  u16 _read_u16();

  ////// The following are generalized Operation functions where the
  // template class T is either an 8-bit register or a LoadHL struct
  // which behaves similarly to an 8-bit register. The main difference
  // is that LoadHL costs 4 cycles to read or write from.  This means
  // we must use a temporary value "u8 v" instead of directly
  // operating on RR to avoid costing the wrong number of cycles.

  // This makes the implementation a bit "magical action"-y, but it
  // seems more obvious to let the cycles speak for themselves rather
  // than manually assigning cycles += 12 for "LD (HL), C" and cycles
  // += 4 for "LD A, C"
  template<class T> inline void INC(T &RR) {
    u8 v = RR + 1;
    RR = v;
    cpu.flags.Z = v == 0;
    cpu.flags.N = 0;
    cpu.flags.H = (v & 0xF) == 0;
  }

  template<class T> inline void DEC(T &RR) {
    u8 v = RR - 1;
    RR = v;
    cpu.flags.Z = v == 0;
    cpu.flags.N = 1;
    cpu.flags.H = (v & 0xF) == 0xF;
  }

  // 8 bit Rotate Right (i.e. x86 ROR)
  template<class T> inline void RRC(T &RR) {
    u8 v = RR;
    u8 v2 = (v >> 1) | (v << 7);
    cpu.registers.F = ((v & 0x01) << 4) | ((v2 == 0) << 7);
    RR = v2;
  }

  // 8 bit Rotate Left
  template<class T> inline void RLC(T &RR) {
    u8 v = RR;
    u8 v2 = (v << 1) | (v >> 7);
    cpu.registers.F = 0;
    cpu.flags.C = v & 0x80;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // 9-bit Rotate Right (i.e. x86 RCR)
  template<class T> inline void RR(T &RR) {
    u8 v = RR;
    u8 v2 = (v >> 1) | (cpu.flags.C << 7);
    cpu.registers.F = 0;
    cpu.flags.C = v & 1;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // 9-bit Rotate Left
  template<class T> inline void RL(T &RR) {
    u8 v = RR;
    u8 v2 = (v << 1) | cpu.flags.C;
    cpu.registers.F = 0;
    cpu.flags.C = v & 0x80;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // Shift Left
  template<class T> inline void SLA(T &RR) {
    u8 v = RR;
    u8 v2 = v << 1;
    cpu.registers.F = 0;
    cpu.flags.C = v & 0x80;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // (Signed) Shift Right
  template<class T> inline void SRA(T &RR) {
    i8 v = RR;
    u8 v2 = v >> 1;
    cpu.registers.F = 0;
    cpu.flags.C = v & 1;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // (Unsigned) Shift Right
  template<class T> inline void SRL(T &RR) {
    u8 v = RR;
    u8 v2 = v >> 1;
    cpu.registers.F = 0;
    cpu.flags.C = v & 1;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  template<class T> inline void SWAP(T &RR) {
    u8 v = RR;
    u8 v2 = (v >> 4) | (v << 4);
    cpu.registers.F = (v == 0) << 7;
    RR = v2;
  }

  void BIT(u8 bit, u8 v) {
    v &= (1 << bit);
    cpu.flags.N = 0;
    cpu.flags.H = 1;
    cpu.flags.Z = v == 0;
  }

  template<class T> void RES(u8 bit, T &RR) { RR = RR & ~(1 << bit); }

  template<class T> void SET(u8 bit, T &RR) { RR = RR | (1 << bit); }

  bool decode();
};

// A value representing the (HL) register. Supports enough
// overloaded operators that it works basically like an 8-bit
// register.
// ====GB asm:===|====C++ code:====
// LD (HL), 0xFF | LoadHL = 0xFF;
// LD B, 0xFF    | B = 0xFF;
// ADD (HL)      | A += LoadHL;
// ADD B         | A += B;
struct MemoryRef {
  Executor & parent;
  CPU::Reg16 &addr;
  operator u8 () const { return parent.mmu_get(addr); }
  MemoryRef& operator=(u8 val) { parent.mmu_set(addr, val); return *this; }
  MemoryRef& operator=(const MemoryRef&) { return *this; }
  void operator++(int) { parent.mmu_set(addr, parent.mmu_get(addr) + 1); }
  void operator--(int) { parent.mmu_set(addr, parent.mmu_get(addr) - 1); }
};
