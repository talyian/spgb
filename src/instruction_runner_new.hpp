#pragma once

#include "base.hpp"
#include "emulator/cpu.hpp"
#include "emulator/mmu.hpp"

// Runs instructions
struct InstructionDasher {
  InstructionDasher(CPU & cpu, MemoryMapper & mmu) : cpu(cpu), mmu(mmu) { }

  CPU & cpu;
  MemoryMapper & mmu;

  u16 PC = 0;
  u16 PC_start = 0;
  u16 PC_next = 0;
  i32 cycles = 0;

  u8 mmu_get(u16 addr) { cycles += 4; return mmu.get(addr); }
  void mmu_set(u16 addr, u8 val) { cycles += 4; mmu.set(addr, val); }

  struct MemoryRef {
    InstructionDasher & parent;
    Reg16 &addr;
    operator u8 () const { return parent.mmu_get(addr); }
    MemoryRef& operator=(u8 val) { parent.mmu_set(addr, val); return *this; }
  };

  u8 _read_u8() {
    cycles += 4;
    return mmu.get(PC++);
  }
  u16 _read_u16() {
    u16 v = _read_u8();
    return v + 256 * _read_u8();
  }

  // 8 bit Rotate Right (i.e. x86 ROR)
  template<class T> // T is either Reg8 or MemoryRef
  inline void RRC(T &RR) {
    u8 v = RR;
    u8 v2 = (v >> 1) | (v << 7);
    cpu.registers.F = ((v & 0x01) << 4) | ((v2 == 0) << 7);
    RR = v2;
  }

  // 8 bit Rotate Left
  template<class T> // T is either Reg8 or MemoryRef
  inline void RLC(T &RR) {
    u8 v = RR;
    u8 v2 = (v << 1) | (v >> 7);
    cpu.registers.F = 0;
    cpu.flags.C = v & 0x80;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // 9-bit Rotate Right (i.e. x86 RCR)
  template<class T> // T is either Reg8 or MemoryRef
  inline void RR(T &RR) {
    u8 v = RR;
    u8 v2 = (RR >> 1) | (cpu.flags.C << 7);
    cpu.registers.F = 0;
    cpu.flags.C = v & 1;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // 9-bit Rotate Left
  template<class T> // T is either Reg8 or MemoryRef
  inline void RL(T &RR) {
    u8 v = RR;
    u8 v2 = (RR << 1) | cpu.flags.C;
    cpu.registers.F = 0;
    cpu.flags.C = v & 0x80;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // Shift Left
  template<class T> // T is either Reg8 or MemoryRef
  inline void SLA(T &RR) {
    u8 v = RR;
    u8 v2 = v << 1;
    cpu.registers.F = 0;
    cpu.flags.C = v & 0x80;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // (Signed) Shift Right
  template<class T> // T is either Reg8 or MemoryRef
  inline void SRA(T &RR) {
    i8 v = RR;
    u8 v2 = v >> 1;
    cpu.registers.F = 0;
    cpu.flags.C = v & 1;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // (Unsigned) Shift Right
  template<class T> // T is either Reg8 or MemoryRef
  inline void SRL(T &RR) {
    u8 v = RR;
    u8 v2 = v >> 1;
    cpu.registers.F = 0;
    cpu.flags.C = v & 1;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // Rotate-4
  template<class T> // T is either Reg8 or MemoryRef
  inline void SWAP(T &RR) {
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
    PC_start = PC;
    u16 opcode = _read_u8();
    if (opcode == 0xCB) opcode = 0x100 + _read_u8();

    // I wonder if there's any difference between these two
    // #define LD16_XXXX(RR) RR = _read_u16()
    #define LD16_XXXX(RR) RR.l = _read_u8(); RR.h = _read_u8()

    Reg8 &A = cpu.registers.A,
      &B = cpu.registers.B,
      &C = cpu.registers.C,
      &D = cpu.registers.D,
      &E = cpu.registers.E,
      &H = cpu.registers.H,
      &L = cpu.registers.L;

    Reg16 &HL = cpu.registers.HL,
      &BC = cpu.registers.BC,
      &DE = cpu.registers.DE,
      &SP = cpu.registers.SP;

    MemoryRef LoadHL {*this, HL};
    
    switch(opcode) {
    case 0x00: /* NOP */; break;
    case 0x01: LD16_XXXX(BC); break;
    case 0x02: mmu_set(BC, A); break;
    case 0x03: BC++; cycles += 4; break;
    case 0x06: B = _read_u8(); break;
    case 0x08: // LD (xxxx), SP
      {
        u16 addr = _read_u16();
        mmu_set(addr++, SP.l);
        mmu_set(addr++, SP.h);
      }
      break;
    case 0x0A: A = mmu_get(BC); break;
    case 0x0E: C = _read_u8(); break;
    case 0x11: LD16_XXXX(DE); break;
    case 0x12: mmu_set(DE, A); break;
    case 0x13: DE++; cycles += 4; break;
    case 0x16: D = _read_u8(); break;
    case 0x1A: A = mmu_get(DE); break;
    case 0x1E: E = _read_u8(); break;
    case 0x21: LD16_XXXX(HL); break;
    case 0x22: mmu_set(HL++, A); break;
    case 0x23: HL++; cycles += 4; break;
    case 0x26: H = _read_u8(); break;
    case 0x2A: A = mmu_get(HL++); break;
    case 0x2E: L = _read_u8(); break;
    case 0x31: LD16_XXXX(SP); break;
    case 0x32: mmu_set(HL--, A); break;
    case 0x33: SP++; cycles += 4; break;
    case 0x36: mmu_set(HL, _read_u8()); break; // LD8 (HL), xx
    case 0x3A: A = mmu_get(HL--); break;
    case 0x3E: A = _read_u8(); break;

    case 0x40: /*B = B;*/ break;
    case 0x41: B = C; break;
    case 0x42: B = D; break;
    case 0x43: B = E; break;
    case 0x44: B = H; break;
    case 0x45: B = L; break;
    case 0x46: B = mmu_get(HL); break;
    case 0x47: B = A; break;

    case 0x48: C = B; break;
    case 0x49: /*C = C;*/ break;
    case 0x4A: C = D; break;
    case 0x4B: C = E; break;
    case 0x4C: C = H; break;
    case 0x4D: C = L; break;
    case 0x4E: C = mmu_get(HL); break;
    case 0x4F: C = A; break;

    case 0x50: D = B; break;
    case 0x51: D = C; break;
    case 0x52: /* D = D; */ break;
    case 0x53: D = E; break;
    case 0x54: D = H; break;
    case 0x55: D = L; break;
    case 0x56: D = mmu_get(HL); break;
    case 0x57: D = A; break;

    case 0x58: E = B; break;
    case 0x59: E = C; break;
    case 0x5A: E = D; break;
    case 0x5B: /* E = E; */ break;
    case 0x5C: E = H; break;
    case 0x5D: E = L; break;
    case 0x5E: E = mmu_get(HL); break;
    case 0x5F: E = A; break;

    case 0x60: H = B; break;
    case 0x61: H = C; break;
    case 0x62: H = D; break;
    case 0x63: H = E; break;
    case 0x64: /*H = H;*/ break;
    case 0x65: H = L; break;
    case 0x66: H = mmu_get(HL); break;
    case 0x67: H = A; break;

    case 0x68: L = B; break;
    case 0x69: L = C; break;
    case 0x6A: L = D; break;
    case 0x6B: L = E; break;
    case 0x6C: L = H; break;
    case 0x6D: /* L = L; */ break;
    case 0x6E: L = mmu_get(HL); break;
    case 0x6F: L = A; break;

    case 0x70: mmu_set(HL, B); break;
    case 0x71: mmu_set(HL, C); break;
    case 0x72: mmu_set(HL, D); break;
    case 0x73: mmu_set(HL, E); break;
    case 0x74: mmu_set(HL, H); break;
    case 0x75: mmu_set(HL, L); break;
    case 0x76: cpu.halted = true; if (cpu.IME == 0) { PC++; } break;
    case 0x77: mmu_set(HL, A); break;

    case 0x78: A = B; break;
    case 0x79: A = C; break;
    case 0x7A: A = D; break;
    case 0x7B: A = E; break;
    case 0x7C: A = H; break;
    case 0x7D: A = L; break;
    case 0x7E: A = mmu_get(HL); break;
    case 0x7F: /* A = A; */ break;
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
    case 0xF9:
      SP = HL;
      cycles += 4;
      break;
    case 0xFA: A = mmu_get(_read_u16()); break;

#define LOOP(F)                                         \
      F(0x0, B); F(0x1, C); F(0x2, D); F(0x3, E);       \
      F(0x4, H); F(0x5, L); F(0x6, LoadHL); F(0x7, A);

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
      // log("unknown op", opcode);
      return false;
    }
    return true;
  }
};
