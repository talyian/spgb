#pragma once

#include "base.hpp"
#include "cpu.hpp"
#include "memory_mapper.hpp"

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
  
  u8 _read_u8() {
    cycles += 4;
    return mmu.get(PC++);
  }
  u16 _read_u16() {
    u16 v = _read_u8();
    return v + 256 * _read_u8();
  }

  // 8 bit Rotate Right (i.e. x86 ROR)
  inline void RRC(Reg8 &RR) {
    u8 v = RR;
    u8 v2 = (v >> 1) | (v << 7);
    cpu.registers.F = ((v & 0x01) << 4) | ((v2 == 0) << 7);
    RR = v2;
  }

  // 8 bit Rotate Left 
  inline void RLC(Reg8 &RR) {
    u8 v = RR;
    u8 v2 = (v << 1) | (v >> 7);
    cpu.registers.F = 0;
    cpu.flags.C = v & 0x80;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // 9-bit Rotate Right (i.e. x86 RCR)
  inline void RR(Reg8 &RR) {
    u8 v = RR;
    u8 v2 = (RR >> 1) | (cpu.flags.C << 7);
    cpu.registers.F = 0;
    cpu.flags.C = v & 1;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // 9-bit Rotate Left
  inline void RL(Reg8 &RR) {
    u8 v = RR;
    u8 v2 = (RR << 1) | cpu.flags.C;
    cpu.registers.F = 0;
    cpu.flags.C = v & 0x80;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // Shift Left
  inline void SLA(Reg8 &RR) {
    u8 v = RR;
    u8 v2 = v << 1;
    cpu.registers.F = 0;
    cpu.flags.C = v & 0x80;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // (Signed) Shift Right
  inline void SRA(Reg8 &RR) {
    i8 v = RR;
    u8 v2 = v >> 1;
    cpu.registers.F = 0;
    cpu.flags.C = v & 1;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // (Unsigned) Shift Right
  inline void SRL(Reg8 &RR) {
    u8 v = RR;
    u8 v2 = v >> 1;
    cpu.registers.F = 0;
    cpu.flags.C = v & 1;
    cpu.flags.Z = v2 == 0;
    RR = v2;
  }

  // Rotate-4
  inline void SWAP(Reg8 &RR) {
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

  void RES(u8 bit, Reg8 &RR) {
    RR = RR & ~(1 << bit);
  }

  void SET(u8 bit, Reg8 &RR) {
    RR = RR | (1 << bit);
  }
  
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

    case 0x100: RLC(B); break;
    case 0x101: RLC(C); break;
    case 0x102: RLC(D); break;
    case 0x103: RLC(E); break;
    case 0x104: RLC(H); break;
    case 0x105: RLC(L); break;
    case 0x106: { Reg8 v = mmu_get(HL); RLC(v); mmu_set(HL, v); break; }
    case 0x107: RLC(A); break;

    case 0x108: RRC(B); break;
    case 0x109: RRC(C); break;
    case 0x10A: RRC(D); break;
    case 0x10B: RRC(E); break;
    case 0x10C: RRC(H); break;
    case 0x10D: RRC(L); break;
    case 0x10E: { Reg8 v = mmu_get(HL); RRC(v); mmu_set(HL, v); break; }
    case 0x10F: RRC(A); break;

    case 0x110: RL(B); break;
    case 0x111: RL(C); break;
    case 0x112: RL(D); break;
    case 0x113: RL(E); break;
    case 0x114: RL(H); break;
    case 0x115: RL(L); break;
    case 0x116: { Reg8 v = mmu_get(HL); RL(v); mmu_set(HL, v); break; }
    case 0x117: RL(A); break;

    case 0x118: RR(B); break;
    case 0x119: RR(C); break;
    case 0x11A: RR(D); break;
    case 0x11B: RR(E); break;
    case 0x11C: RR(H); break;
    case 0x11D: RR(L); break;
    case 0x11E: { Reg8 v = mmu_get(HL); RR(v); mmu_set(HL, v); break; }
    case 0x11F: RR(A); break;

    case 0x120: SLA(B); break;
    case 0x121: SLA(C); break;
    case 0x122: SLA(D); break;
    case 0x123: SLA(E); break;
    case 0x124: SLA(H); break;
    case 0x125: SLA(L); break;
    case 0x126: { Reg8 v = mmu_get(HL); SLA(v); mmu_set(HL, v); break; }
    case 0x127: SLA(A); break;

    case 0x128: SRA(B); break;
    case 0x129: SRA(C); break;
    case 0x12A: SRA(D); break;
    case 0x12B: SRA(E); break;
    case 0x12C: SRA(H); break;
    case 0x12D: SRA(L); break;
    case 0x12E: { Reg8 v = mmu_get(HL); SRA(v); mmu_set(HL, v); break; }
    case 0x12F: SRA(A); break;

    case 0x130: SWAP(B); break;
    case 0x131: SWAP(C); break;
    case 0x132: SWAP(D); break;
    case 0x133: SWAP(E); break;
    case 0x134: SWAP(H); break;
    case 0x135: SWAP(L); break;
    case 0x136: { Reg8 v = mmu_get(HL); SWAP(v); mmu_set(HL, v); break; }
    case 0x137: SWAP(A); break;

    case 0x138: SRL(B); break;
    case 0x139: SRL(C); break;
    case 0x13A: SRL(D); break;
    case 0x13B: SRL(E); break;
    case 0x13C: SRL(H); break;
    case 0x13D: SRL(L); break;
    case 0x13E: { Reg8 v = mmu_get(HL); SRL(v); mmu_set(HL, v); break; }
    case 0x13F: SRL(A); break;

    case 0x140: BIT(0, B); break;
    case 0x141: BIT(0, C); break;
    case 0x142: BIT(0, D); break;
    case 0x143: BIT(0, E); break;
    case 0x144: BIT(0, H); break;
    case 0x145: BIT(0, L); break;
    case 0x146: BIT(0, mmu_get(HL)); cycles += 4; break;
    case 0x147: BIT(0, A); break;

    case 0x148: BIT(1, B); break;
    case 0x149: BIT(1, C); break;
    case 0x14A: BIT(1, D); break;
    case 0x14B: BIT(1, E); break;
    case 0x14C: BIT(1, H); break;
    case 0x14D: BIT(1, L); break;
    case 0x14E: BIT(1, mmu_get(HL)); cycles += 4; break;
    case 0x14F: BIT(1, A); break;

    case 0x150: BIT(2, B); break;
    case 0x151: BIT(2, C); break;
    case 0x152: BIT(2, D); break;
    case 0x153: BIT(2, E); break;
    case 0x154: BIT(2, H); break;
    case 0x155: BIT(2, L); break;
    case 0x156: BIT(2, mmu_get(HL)); cycles += 4; break;
    case 0x157: BIT(2, A); break;

    case 0x158: BIT(3, B); break;
    case 0x159: BIT(3, C); break;
    case 0x15A: BIT(3, D); break;
    case 0x15B: BIT(3, E); break;
    case 0x15C: BIT(3, H); break;
    case 0x15D: BIT(3, L); break;
    case 0x15E: BIT(3, mmu_get(HL)); cycles += 4; break;
    case 0x15F: BIT(3, A); break;

    case 0x160: BIT(4, B); break;
    case 0x161: BIT(4, C); break;
    case 0x162: BIT(4, D); break;
    case 0x163: BIT(4, E); break;
    case 0x164: BIT(4, H); break;
    case 0x165: BIT(4, L); break;
    case 0x166: BIT(4, mmu_get(HL)); cycles += 4; break;
    case 0x167: BIT(4, A); break;

    case 0x168: BIT(5, B); break;
    case 0x169: BIT(5, C); break;
    case 0x16A: BIT(5, D); break;
    case 0x16B: BIT(5, E); break;
    case 0x16C: BIT(5, H); break;
    case 0x16D: BIT(5, L); break;
    case 0x16E: BIT(5, mmu_get(HL)); cycles += 4; break;
    case 0x16F: BIT(5, A); break;

    case 0x170: BIT(6, B); break;
    case 0x171: BIT(6, C); break;
    case 0x172: BIT(6, D); break;
    case 0x173: BIT(6, E); break;
    case 0x174: BIT(6, H); break;
    case 0x175: BIT(6, L); break;
    case 0x176: BIT(6, mmu_get(HL)); cycles += 4; break;
    case 0x177: BIT(6, A); break;

    case 0x178: BIT(7, B); break;
    case 0x179: BIT(7, C); break;
    case 0x17A: BIT(7, D); break;
    case 0x17B: BIT(7, E); break;
    case 0x17C: BIT(7, H); break;
    case 0x17D: BIT(7, L); break;
    case 0x17E: BIT(7, mmu_get(HL)); cycles += 4; break;
    case 0x17F: BIT(7, A); break;
      
    default:
      // log("unknown op", opcode);
      return false;
    }
    return true;
  }
};
