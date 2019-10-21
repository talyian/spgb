#pragma once

#include "base.hpp"
#include "system/cpu.hpp"
#include "system/mmu.hpp"

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

  u8 mmu_get(u16 addr) { cycles += 4; return mmu.get(addr); }
  void mmu_set(u16 addr, u8 val) { cycles += 4; mmu.set(addr, val); }
  u16 _pop() {
    // total 8 cycles
    u16 v = mmu_get(cpu.registers.SP++);
    return v + (mmu_get(cpu.registers.SP++) << 8);
  }
  void _push(u16 value) {
    // total 12 cycles
    mmu_set(--cpu.registers.SP, value >> 8);
    mmu_set(--cpu.registers.SP, value);
    cycles += 4;
  }

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

  u8 _read_u8() {
    cycles += 4;
    return mmu.get(PC++);
  }
  u16 _read_u16() {
    u16 v = _read_u8();
    return v + 256 * _read_u8();
  }

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

  template<class T> // T is either Reg8 or MemoryRef
  void RES(u8 bit, T &RR) { RR = RR & ~(1 << bit); }

  template<class T> // T is either Reg8 or MemoryRef
  void SET(u8 bit, T &RR) { RR = RR | (1 << bit); }

  bool decode() {
    PC_start = PC; PC_next = 0;
    u16 opcode = _read_u8();
    if (opcode == 0xCB) opcode = 0x100 + _read_u8();

    #define LD16_XXXX(RR) RR.l = _read_u8(); RR.h = _read_u8()
    #define ADD(R) {u8 a = A, b = R; \
      A = a + b; \
      cpu.flags.Z = (u8)(a + b) == 0; \
      cpu.flags.C = (u16)a + (u16)b > 0xFF; \
      cpu.flags.H = (a & 0xF) + (b & 0xF) > 0xF; \
      cpu.flags.N = 0;}
    #define ADC(R) {u8 a = A, b = R, c = cpu.flags.C; \
      A = a + b + c; \
      cpu.flags.Z = (u8)(a + b + c) == 0; \
      cpu.flags.C = (a + b + c) > 0xFF; \
      cpu.flags.H = (a & 0xF) + (b & 0xF) + c > 0xF; \
      cpu.flags.N = 0;}
    #define SUB(R) {u8 a = A, b = R;            \
      A = a - b;                                \
      cpu.flags.Z = a == b;                     \
      cpu.flags.C = a < b;                      \
      cpu.flags.H = (a & 0xF) < (b & 0xF);      \
      cpu.flags.N = 1;}
    #define SBC(R) {u8 a = A, b = R, c = cpu.flags.C; \
      A = a - b - c; \
      cpu.flags.Z = a == (u8)(b + c); \
      cpu.flags.N = 1; \
      cpu.flags.C = (u16) a < (u16) b + c; \
      cpu.flags.H = (a & 0xF) < (b & 0xF) + c; }
    #define AND(R) {A = A & R; cpu.registers.F = 0x20 | ((A == 0) << 7); }
    #define XOR(R) {A = A ^ R; cpu.registers.F = ((A == 0) << 7); }
    #define  OR(R) {A = A | R; cpu.registers.F = ((A == 0) << 7); }
    #define  CP(R) {u8 a = A, b = R;            \
      cpu.flags.Z = a == b;                     \
      cpu.flags.C = a < b;                      \
      cpu.flags.H = (a & 0xF) < (b & 0xF);      \
      cpu.flags.N = 1;}

    CPU::Reg8 &A = cpu.registers.A,
      &B = cpu.registers.B,
      &C = cpu.registers.C,
      &D = cpu.registers.D,
      &E = cpu.registers.E,
      &H = cpu.registers.H,
      &L = cpu.registers.L,
      &F = cpu.registers.F;

    CPU::Reg16 &HL = cpu.registers.HL,
      &BC = cpu.registers.BC,
      &DE = cpu.registers.DE,
      &SP = cpu.registers.SP;

    MemoryRef LoadHL {*this, HL};

#define LOOP0(X)                                         \
      X(0x0, B); X(0x1, C); X(0x2, D); X(0x3, E);       \
      X(0x4, H); X(0x5, L); X(0x7, A);
#define LOOP(X) LOOP0(X) X(0x6, LoadHL);
    switch(opcode) {
    case 0x00: /* NOP */; break;
    case 0x10: cpu.stopped = cpu.halted = 1; break;
    case 0x01: LD16_XXXX(BC); break;
    case 0x11: LD16_XXXX(DE); break;
    case 0x21: LD16_XXXX(HL); break;
    case 0x31: LD16_XXXX(SP); break;
    #undef LD16_XXXX
    case 0x02: mmu_set(BC, A); break;

    #define X(OP, R)                     \
      case 0x04 + 8 * OP: INC(R); break; \
      case 0x05 + 8 * OP: DEC(R); break; \
      case 0x06 + 8 * OP: R = _read_u8(); break;
    LOOP(X)
    #undef X

    case 0x07: RLC(A); cpu.flags.Z = 0; break;
    case 0x0F: RRC(A); cpu.flags.Z = 0; break;
    case 0x17: RL(A); cpu.flags.Z = 0; break;
    case 0x1F: RR(A); cpu.flags.Z = 0; break;

    case 0x08: // LD (xxxx), SP
      {
        u16 addr = _read_u16();
        mmu_set(addr++, SP.l);
        mmu_set(addr++, SP.h);
      }
      break;
    case 0x0A: A = mmu_get(BC); break;
    case 0x12: mmu_set(DE, A); break;

    case 0x03: BC++; cycles += 4; break;
    case 0x13: DE++; cycles += 4; break;
    case 0x23: HL++; cycles += 4; break;
    case 0x33: SP++; cycles += 4; break;

    case 0x0B: BC--; cycles += 4; break;
    case 0x1B: DE--; cycles += 4; break;
    case 0x2B: HL--; cycles += 4; break;
    case 0x3B: SP--; cycles += 4; break;

    case 0x1A: A = mmu_get(DE); break;
    case 0x22: mmu_set(HL++, A); break;
    case 0x2A: A = mmu_get(HL++); break;
    case 0x32: mmu_set(HL--, A); break;
    case 0x3A: A = mmu_get(HL--); break;

    case 0xF3: cpu.IME = 0; break;
    case 0xFB: cpu.IME = 1; break;
    case 0x27: /* DAA */ {
      if (cpu.flags.N) {
        if (cpu.flags.C) { A -= 0x60; } // N+C: borrow a 10 digit
        if (cpu.flags.H) { A -= 0x06; } // N+H: borrow a 01 digit
      } else {
        if (cpu.flags.C || A > 0x99) { A += 0x60; cpu.flags.C = 1; } // C: carry a 10 digit
        if (cpu.flags.H || (A & 0xF) > 0x09) { A += 0x06; } // H: carry a 01 digit
      }
      cpu.flags.Z = A == 0;
      cpu.flags.H = 0;
      break;
    }
    case 0x2F: A = ~A; F |= 0x60; break;             // Complement A
    case 0x37: F = (F & 0x80) | 0x10; break;        // Set Carry
    case 0x3F: F = (F & 0x80) | (~F & 0x10); break; // Complement Carry

    case 0xC7: _push(PC); PC = 0x00; break;
    case 0xCF: _push(PC); PC = 0x08; break;
    case 0xD7: _push(PC); PC = 0x10; break;
    case 0xDF: _push(PC); PC = 0x18; break;
    case 0xE7: _push(PC); PC = 0x20; break;
    case 0xEF: _push(PC); PC = 0x28; break;
    case 0xF7: _push(PC); PC = 0x30; break;
    case 0xFF: _push(PC); PC = 0x38; break;

    #define X(OP, REG) \
      case 0x40 + OP: B = REG; break; \
      case 0x48 + OP: C = REG; break; \
      case 0x50 + OP: D = REG; break; \
      case 0x58 + OP: E = REG; break; \
      case 0x60 + OP: H = REG; break; \
      case 0x68 + OP: L = REG; break; \
      case 0x78 + OP: A = REG; break;
    LOOP(X)
    #undef X
    #define X(OP, REG) \
      case 0x70 + OP: LoadHL = REG; break;
    LOOP0(X)
    #undef X
    case 0x76: cpu.halted = true; if (cpu.IME == 0) { PC++; } break;
    #define ADD_HL(RR) {u16 a = HL, b = RR; \
      cycles += 4; \
      HL = a + b; \
      cpu.flags.N = 0; \
      cpu.flags.C = a > (u16)~b; \
      cpu.flags.H = (a & 0xFFF) + (b & 0xFFF) > 0xFFF;}
    case 0x09: ADD_HL(BC); break;
    case 0x19: ADD_HL(DE); break;
    case 0x29: ADD_HL(HL); break;
    case 0x39: ADD_HL(SP); break;
    #define X(OP, REG) \
      case 0x80 + OP: ADD(REG); break; \
      case 0x88 + OP: ADC(REG); break; \
      case 0x90 + OP: SUB(REG); break; \
      case 0x98 + OP: SBC(REG); break; \
      case 0xA0 + OP: AND(REG); break; \
      case 0xA8 + OP: XOR(REG); break; \
      case 0xB0 + OP:  OR(REG); break; \
      case 0xB8 + OP:  CP(REG); break;
    LOOP(X)
    #undef X

    case 0xC6: ADD(_read_u8()); break;
    case 0xD6: SUB(_read_u8()); break;
    case 0xE6: AND(_read_u8()); break;
    case 0xF6:  OR(_read_u8()); break;
    case 0xCE: ADC(_read_u8()); break;
    case 0xDE: SBC(_read_u8()); break;
    case 0xEE: XOR(_read_u8()); break;
    case 0xFE:  CP(_read_u8()); break;

#define CALL(OP, COND) case OP: { u16 target = _read_u16(); if (COND) { _push(PC); PC_next = PC; PC = target;} break; }
    CALL(0xC4, !cpu.flags.Z)
    CALL(0xCC, cpu.flags.Z)
    CALL(0xCD, true)
    CALL(0xD4, !cpu.flags.C)
    CALL(0xDC, cpu.flags.C)
      // RET - 20/8/16 cycles
    case 0xC0: { cycles += 4; if (!cpu.flags.Z) { PC = _pop(); cycles += 4; } break; }
    case 0xC8: { cycles += 4; if (cpu.flags.Z) { PC = _pop(); cycles += 4; } break; }
    case 0xC9: { cycles += 4; if (true)         { PC = _pop(); } break; }
    case 0xD0: { cycles += 4; if (!cpu.flags.C) { PC = _pop(); cycles += 4; } break; }
    case 0xD8: { cycles += 4; if (cpu.flags.C) { PC = _pop(); cycles += 4; } break; }
    case 0xD9: { cycles += 4; if (true)         { cpu.IME = 1; PC = _pop(); } break; }
      // JP
    #define JP(OP, COND) case OP: { u16 target = _read_u16(); if (COND) { PC_next = PC; PC = target; cycles += 4; } break; }
    JP(0xC2, !cpu.flags.Z);
    JP(0xC3, true);
    JP(0xCA, cpu.flags.Z);
    JP(0xD2, !cpu.flags.C);
    JP(0xDA, cpu.flags.C);
    #undef JP
    #undef CALL
    case 0xE9: { PC = HL; break; }
      // JR
    case 0x20: {i8 o = _read_u8(); if (!cpu.flags.Z) { PC += o; cycles += 4; }} break;
    case 0x30: {i8 o = _read_u8(); if (!cpu.flags.C) { PC += o; cycles += 4; }} break;
    case 0x18: {i8 o = _read_u8(); if (true)         { PC += o; cycles += 4; }} break;
    case 0x28: {i8 o = _read_u8(); if (cpu.flags.Z)  { PC += o; cycles += 4; }} break;
    case 0x38: {i8 o = _read_u8(); if (cpu.flags.C)  { PC += o; cycles += 4; }} break;

    case 0xC1: BC = _pop(); break;
    case 0xD1: DE = _pop(); break;
    case 0xE1: HL = _pop(); break;
    case 0xF1: cpu.registers.AF = 0xFFF0 & _pop(); break;
    case 0xC5: _push(BC); break;
    case 0xD5: _push(DE); break;
    case 0xE5: _push(HL); break;
    case 0xF5: _push(cpu.registers.AF); break;

    case 0xE0: mmu_set(0xFF00 + _read_u8(), A); break; // LD8 IO+u8, A
    case 0xE2: mmu_set(0xFF00 + C, A); break; // LD8 IO+C, A
    case 0xEA: mmu_set(_read_u16(), A); break; // LD8 xxxx, A
    case 0xE8: // LD SP, SP+i8
      {
        u16 sp = SP;
        i8 offset = _read_u8();
        cpu.registers.F = 0;
        cpu.flags.H = (sp & 0xF) + (offset & 0xF) > 0xF;
        cpu.flags.C = (sp & 0xFF) + (offset & 0xFF) > 0xFF;
        SP = sp + offset;
      }
      cycles += 8; // total 16 cycles
      break;
    case 0xF0: A = mmu_get(0xFF00 + _read_u8()); break;
    case 0xF2: A = mmu_get(0xFF00 + C); break;
    case 0xF8: // LD HL, SP+i8
      {
        u16 sp = SP;
        i8 offset = _read_u8();
        cpu.registers.F = 0;
        cpu.flags.H = (sp & 0xF) + (offset & 0xF) > 0xF;
        cpu.flags.C = (sp & 0xFF) + (offset & 0xFF) > 0xFF;
        cpu.registers.HL = sp + offset;
      }
      cycles += 4; // total 12 cycles
      break;
    case 0xF9: SP = HL; cycles += 4; break;
    case 0xFA: A = mmu_get(_read_u16()); break;

    #define X(op, target) \
      case 0x100 + op: RLC(target); break;  \
      case 0x108 + op: RRC(target); break;  \
      case 0x110 + op: RL(target); break;   \
      case 0x118 + op: RR(target); break;   \
      case 0x120 + op: SLA(target); break;  \
      case 0x128 + op: SRA(target); break;  \
      case 0x130 + op: SWAP(target); break; \
      case 0x138 + op: SRL(target); break;
    LOOP(X)
    #undef X
    #define X(op, target) \
      case 0x140 + op: BIT(0, target); break;\
      case 0x148 + op: BIT(1, target); break;\
      case 0x150 + op: BIT(2, target); break;\
      case 0x158 + op: BIT(3, target); break;\
      case 0x160 + op: BIT(4, target); break;\
      case 0x168 + op: BIT(5, target); break;\
      case 0x170 + op: BIT(6, target); break;\
      case 0x178 + op: BIT(7, target); break;
    LOOP(X)
    #undef X
    #define X(op, target) \
      case 0x180 + op: RES(0, target); break;\
      case 0x188 + op: RES(1, target); break;\
      case 0x190 + op: RES(2, target); break;\
      case 0x198 + op: RES(3, target); break;\
      case 0x1A0 + op: RES(4, target); break;\
      case 0x1A8 + op: RES(5, target); break;\
      case 0x1B0 + op: RES(6, target); break;\
      case 0x1B8 + op: RES(7, target); break;
    LOOP(X)
    #undef X
    #define X(op, target) \
      case 0x1C0 + op: SET(0, target); break;\
      case 0x1C8 + op: SET(1, target); break;\
      case 0x1D0 + op: SET(2, target); break;\
      case 0x1D8 + op: SET(3, target); break;\
      case 0x1E0 + op: SET(4, target); break;\
      case 0x1E8 + op: SET(5, target); break;\
      case 0x1F0 + op: SET(6, target); break;\
      case 0x1F8 + op: SET(7, target); break;
    LOOP(X)
    #undef X
    default:
      log(PC_start, "unknown op", opcode);
      error = 1;
      return false;
    }
    return true;
  }
};
