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

  
  bool decode() {
    PC_start = PC;
    u16 opcode = _read_u8();
    if (opcode == 0xCB) opcode = 0x100 + _read_u8();

#define LD16_XXXX(RR) RR = _read_u16()
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
    case 0x16: D = _read_u8(); break;
    case 0x1A: A = mmu_get(DE); break;
    case 0x1E: E = _read_u8(); break;
    case 0x21: LD16_XXXX(HL); break;
    case 0x22: mmu_set(HL++, A); break;
    case 0x26: H = _read_u8(); break;
    case 0x2A: A = mmu_get(HL++); break;
    case 0x2E: L = _read_u8(); break;
    case 0x31: LD16_XXXX(SP); break;
    case 0x32: mmu_set(HL--, A); break;      
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
    default:
      // log("unknown op", opcode);
      return false;
    }
    return true;
  }
};
