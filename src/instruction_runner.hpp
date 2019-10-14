#pragma once
#include "instructions.hpp"
#include "platform.hh"
#include "memory_mapper.hpp"
#include "cpu.hpp"
  
struct InstructionRunner {
  InstructionRunner(CPU &cpu) : cpu(cpu) {  }
  
  CPU &cpu;
  
  u16 *PC_ptr;
  u16 *PC_start_ptr;
  u16 cycles = 0;
  
  int error = 0;
  int verbose_log = 0;
  MemoryMapper *mmu = 0;

  void dump() {
    log(". . . . . . . . . A  F  B  C  D  E  HL   SP   PC");
    log(". . . . . . . . .", cpu.registers.A, cpu.registers.F,
        cpu.registers.B, cpu.registers.C,
        cpu.registers.D, cpu.registers.E,
        cpu.registers.HL, cpu.registers.SP, *PC_start_ptr);
  }
  
  template<class T, class ...TS>
  void m_log(T x, TS ... xs) {
    // if (verbose_log || error)
    //   log(x, xs...);
  }
  
  void _push(Reg16 value) {
    mmu->set(--cpu.registers.SP, value.h);
    mmu->set(--cpu.registers.SP, value.l);
  }
  u16 _pop() {
    u16 l = mmu->get(cpu.registers.SP++);
    u16 h = mmu->get(cpu.registers.SP++);
    return h * 0x100 + l;
  }
  
  u8 _read8_addr(u16 addr) {
    // if (0xFF00 <= addr && addr < 0xFF80 && addr != 0xFF44 && addr != 0xFF42)
    //   log("    IO:", addr);
    return mmu->get(addr);
  }
  u8 _read8(Register8 r) {
    switch(r) {
#define X(RR)     case Register8::RR: return cpu.registers.RR; break;
      X(A);
      X(B);
      X(C);
      X(D);
      X(E);
      X(F);
      X(H);
      X(L);
#undef X
    }
  }
  
  u8 _read8(Value8 v) {
    switch(v.type) {
    case Value8::IMM8: return v.value;
    case Value8::REG8: return _read8(v.reg);
    case Value8::Ld8Reg: return _read8_addr(_read16(v.reg16));
    case Value8::Ld8Imm: return _read8_addr(v.addr);
    case Value8::IoImm8: return _read8_addr(0xFF00 + v.value);
    case Value8::IoReg8: return _read8_addr(0xFF00 + _read8(v.reg));
    case Value8::Ld8Dec: {
      auto addr = _read16(v.reg16);
      auto value = _read8_addr(addr);
      _write16(v.reg16, addr - 1);
      return value; }
    case Value8::Ld8Inc: {
      auto addr = _read16(v.reg16);
      auto value = _read8_addr(addr);
      _write16(v.reg16, addr + 1);
      return value; }
    }
  }
  
  void _write8_addr(u16 addr, u8 value) {
    // sprite table 1
    // if (0x8000 <= addr && addr < 0x8800) {
    //   log(*PC_start_ptr, addr, "<-", value);
    // }
    
    // if (*PC_start_ptr > 0x100) { 
    // // if (addr == 0xFF42) { log(*PC_start_ptr, "scroll y", value); }
    // // if (addr == 0xFF43) { log(*PC_start_ptr, "scroll x", value); }
    // }
    if (0xFF00 <= addr && addr < 0xFF80) _handle_io_write(addr, value);
    mmu->set(addr, value);
  }
  void _write8(Register8 target, u8 value) {
    switch(target) {
#define X(RR)     case Register8::RR: cpu.registers.RR = value; break;
      X(A);
      X(B);
      X(C);
      X(D);
      X(E);
      X(F);
      X(H);
      X(L);
#undef X
    }
  }

  void _write8(Value8 target, u8 value) {
    switch(target.type) {
    case Value8::IMM8: { log("error-write-to-imm"); error = 100; return; }
    case Value8::REG8: {
      _write8(target.reg, value);
      return;
    }
    case Value8::IoImm8: { _write8_addr(0xFF00 + target.value, value); return; }
    case Value8::IoReg8: { _write8_addr(0xFF00 + _read8(target.reg), value); return; }
    case Value8::Ld8Reg: { _write8_addr(_read16(target.reg16), value); return; }
    case Value8::Ld8Inc: {
      u16 addr = _read16(target.reg16);
      _write8_addr(addr, value);
      _write16(target.reg16, addr + 1);
      return;
    }
    case Value8::Ld8Dec: {
      u16 addr = _read16(target.reg16);
      _write8_addr(addr, value);
      _write16(target.reg16, addr - 1);
      return;
    }
    case Value8::Ld8Imm: { _write8_addr(target.addr, value); return; }
    }
  }
  
  u16 _read16_addr(u16 addr) {
    u16 value = mmu->get(addr++);
    return value + mmu->get(addr) * 0x100;
  }
  u16 _read16(Register16 r) {
    switch(r) {
    case Register16::BC: return cpu.registers.BC; break;
    case Register16::DE: return cpu.registers.DE; break;
    case Register16::HL: return cpu.registers.HL; break;
    case Register16::SP: return cpu.registers.SP; break;
    case Register16::AF: return cpu.registers.AF; break;
    }
  }
  u16 _read16(Value16 v) {
    switch(v.type) {
    case Value16::IMM16: return v.value;
    case Value16::REG16: return _read16(v.reg);
    case Value16::SP_d8: {
      u16 sp = cpu.registers.SP;
      u16 offset = (i8)v.offset;
      u16 sp_2 = sp + offset;
      cpu.flags.Z = 0;
      cpu.flags.N = 0;
      cpu.flags.H = (sp & 0xF) + (offset & 0xF) > 0xF;
      cpu.flags.C = ((sp & 0xFF) + (v.offset & 0xFF)) > 0xFF;
      return sp + (i8)v.offset;
    }}
  }
  
  void _write16(Register16 r, u16 value) {
    switch(r) {
    case Register16::BC: cpu.registers.BC = value; break;
    case Register16::DE: cpu.registers.DE = value; break;
    case Register16::HL: cpu.registers.HL = value; break;
    case Register16::SP: cpu.registers.SP = value; break;
    case Register16::AF: cpu.registers.AF = value & 0xFFF0; break;
    }
  }

  void _write16_addr(u16 addr, u16 value) {
    mmu->set(addr++, value);
    mmu->set(addr, value >> 8);
  }

  void _write16(Value16 target, u16 value) {
    switch(target.type) {
    case Value16::IMM16: return _write16_addr(target.value, value);
    case Value16::REG16: return _write16(target.reg, value);
    case Value16::SP_d8:
      return _write16_addr((u16)((u16)cpu.registers.SP + (i8)target.offset), value);
    }
  }

  void _handle_io_write(u16 addr, u16 value) {
// #define WARN(msg) log("IO CTL", msg, value); break;
#define WARN(msg) break;
    switch(addr) {
    // joypad
    case 0xFF00: break;
    // serial control
    case 0xFF01: break;
    case 0xFF02:
      if (value & 0x80) {
        cpu.serial.out_buf[cpu.serial.pos++] = mmu->get(0xFF01);
        mmu->set(0xFF02, value & ~0x80);
      }
      break;
    // case 0xFF0F: log(*PC_start_ptr, "     (IF) IO:", addr, "<-", value); break;
    case 0xFF04: 
    case 0xFF05: 
    case 0xFF06:
    case 0xFF07: WARN("timer");

    case 0xFF0F: WARN("interrupt");
    // case 0xFF10:
    // case 0xFF11:
    // case 0xFF12:
    // case 0xFF13:
    // case 0xFF14:
    // case 0xFF16:
    // case 0xFF17:
    // case 0xFF18:
    // case 0xFF19:
    // case 0xFF1A:
    // case 0xFF1B:
    // case 0xFF1C:
    // case 0xFF1D:
    // case 0xFF1E:
    // case 0xFF20:
    // case 0xFF21:
    // case 0xFF22:
    // case 0xFF23:
    // case 0xFF24:
    // case 0xFF25:
    // case 0xFF26:
    //   WARN("sound");
    case 0xFF40: WARN("LCDC"); break;
    case 0xFF41: WARN("stat"); break;
    case 0xFF42: 
    case 0xFF43: 
    case 0xFF44: 
    case 0xFF45:
    case 0xFF4A:
    case 0xFF4B: WARN("screen");
      
    case 0xFF46: /* DMA */ break;
    case 0xFF47: /* BG Palette */ break;
    case 0xFF48: /* BG Palette */ break;
    case 0xFF49: /* BG Palette */ break;
      // case 0xFF4D: 
    case 0xFF4F: /* hblank dma */ break;
    case 0xFF50: break;
    case 0xFF68:
    case 0xFF69:
    case 0xFF6A: WARN("CGB pal"); 
    case 0xFFFF: /* IE */; break;
    default:
      if (0xFF10 <= addr && addr < 0xFF40) WARN("sound");
      log(*PC_start_ptr, "Unhandled IOCTL", addr, value);
      // _stop();
    }
  }

  void NOP() {

  }
  
  void STOP() {
    cpu.stopped = 1;
    cpu.halted = 1;
  }
  void DAA() {
    auto &fl = cpu.flags;
    auto &registers = cpu.registers;
    if (fl.N) {
      if (fl.C) { registers.A -= 0x60; }
      if (fl.H) { registers.A -= 0x06; }
    }
    else {
      if (fl.C || registers.A > 0x99) { registers.A += 0x60; fl.C = 1; }
      if (fl.H || (registers.A & 0xF) > 0x9) { registers.A += 0x06; }
    }
    fl.Z = registers.A == 0;
    fl.H = 0;
  }

  // Complement A
  void CPL() {
    auto &fl = cpu.flags;
    auto &registers = cpu.registers;
    registers.A = ~registers.A;
    fl.N = 1;
    fl.H = 1;
  }

  // Set Carry Flag
  void SCF() {
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    cpu.flags.C = 1;
  }

  // Complement Carry Flag
  void CCF() {
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    cpu.flags.C = 1 - cpu.flags.C;
  }

  // Disable Interrupts
  void DI() {
    cpu.IME = 0;
  }

  // Enable Interrupts
  void EI() {
    cpu.IME = 1;
  }

  void HALT() {
    cpu.halted = 1;
    if (cpu.IME == 0) {
      *PC_ptr += 1;
    }
  }

  void RLCA() {
    RLC(Register8::A);
    cpu.flags.Z = 0;
  }
  void RRCA() {
    RRC(Register8::A);
    cpu.flags.Z = 0;
  }
  void RLA() {
    RL(Register8::A);
    cpu.flags.Z = 0;
  }
  void RRA() {
    RR(Register8::A);
    cpu.flags.Z = 0;
  }
  
  void LD8(Value8 o, Value8 v) {
    _write8(o, _read8(v));
  }
  void LD16(Value16 o, Value16 v) {
    _write16(o, _read16(v));
  }

  void BIT(u8 o, Value8 v) {
    if (o < 0 || o > 7) { log("bit", o, v); error = 3; return; }
    u8 val = _read8(v) & (1 << o);
    cpu.flags.Z = val == 0;
    cpu.flags.N = 0;
    cpu.flags.H = 1;
  }

  void RES(u8 o, Value8 v) {
    if (o < 0 || o > 7) { log("bit", o, v); error = 3; return; }
    u8 val = _read8(v) & ~(1 << o);
    _write8(v, val);
  }
  void SET(u8 o, Value8 v) {
    if (o < 0 || o > 7) { log("bit", o, v); error = 3; return; }
    u8 val = _read8(v) | (1 << o);
    _write8(v, val);
  }
  
  void ADD(Value16 o, Value16 v) {
    // TODO: addSP has different flags and timing!
    u16 a = _read16(o);
    u16 b = _read16(v);
    _write16(o, a + b);
    cpu.flags.N = 0;
    cpu.flags.C = a > (u16)~b;
    cpu.flags.H = (a & 0xFFF) + (b & 0xFFF) > 0xFFF;
  }

  void ADD(Value8 o, Value8 v) {
    u8 a = _read8(o);
    u8 b = _read8(v);
    _write8(o, a + b);
    cpu.flags.Z = (u8)(a + b) == 0;
    cpu.flags.C = (u16)a + (u16)b > 0xFF;
    cpu.flags.H = (a & 0xF) + (b & 0xF) > 0xF;
    cpu.flags.N = 0;
  }

  void ADC(Value8 o, Value8 v) {
    u8 a = _read8(o);
    u8 b = _read8(v);
    u8 c = cpu.flags.C;
    u16 d = a + b + c;
    _write8(o, d);
    cpu.flags.Z = (u8)d == 0;
    cpu.flags.C = d > 0xFF;
    cpu.flags.H = (a & 0xF) + (b & 0xF) + c > 0xF;
    cpu.flags.N = 0;
  }

  void XOR(Value8 o) {
    cpu.registers.A = cpu.registers.A ^ _read8(o);
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    cpu.flags.C = 0;
    cpu.flags.Z = cpu.registers.A == 0;
  }
  void AND(Value8 o) {
    cpu.registers.A = cpu.registers.A & _read8(o);
    cpu.flags.N = 0;
    cpu.flags.H = 1;
    cpu.flags.C = 0;
    cpu.flags.Z = cpu.registers.A == 0;
  }
  void OR(Value8 o) {
    cpu.registers.A = cpu.registers.A | _read8(o);
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    cpu.flags.C = 0;
    cpu.flags.Z = cpu.registers.A == 0;
  }

  void DEC(Value8 o) {
    u8 a = _read8(o);
    u8 v = a - (u8)1;
    _write8(o, v);
    cpu.flags.Z = a == 1;
    cpu.flags.N = 1;
    cpu.flags.H = (a & 0xF) < 1;
  } 
  void DEC(Register16 o) {
    u16 v = _read16(o);
    _write16(o, v - 1);
    // no flags
  }
  
  void INC(Value8 o) {
    u8 v = _read8(o);
    _write8(o, v + 1);
    cpu.flags.N = 0;
    cpu.flags.H = (v & 0xF) == 0xF;
    cpu.flags.Z = v == 0xFF;
  }  // INC LoadHL)

  void INC(Register16 o) {
    u16 v = _read16(o) + 1;
    _write16(o, v);
  } // INC HL

  void SUB(Value8 o) {
    u8 a = _read8(Register8::A);
    u8 b = _read8(o);
    _write8(Register8::A, a - b);
    cpu.flags.Z = a == b;
    cpu.flags.N = 1;
    cpu.flags.C = a < b;
    cpu.flags.H = (a & 0xF) < (b & 0xF);
  }

  void SBC(Value8 o) {
    u8 a = _read8(Register8::A);
    u8 b = _read8(o);
    u8 c = cpu.flags.C;
    u8 d = a - b - c;
    _write8(Register8::A, d);
    cpu.flags.Z = a == (u8)(b + c);
    cpu.flags.N = 1;
    cpu.flags.C = (u16)a < (u16)b + c;
    cpu.flags.H = (a & 0xF) < (b & 0xF) + c;
  }

  // unsigned right-shift
  void SRL(Value8 o) {
    u8 v = _read8(o);
    cpu.flags.C = v & 1;
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    cpu.flags.Z = (v >> 1) == 0;
    _write8(o, v >> 1);
  }

  // signed right-shift
  void SRA(Value8 o) {
    u8 v = _read8(o);
    cpu.flags.C = v & 1;
    cpu.flags.H = 0;
    cpu.flags.N = 0;
    cpu.flags.Z = (((i8)v) >> 1) == 0;
    _write8(o, ((i8)v) >> 1);
  }

  // left shift
  void SLA(Value8 o) {
    u8 v = _read8(o);
    u8 v2 = v << 1;
    cpu.flags.C = v & 0x80;
    cpu.flags.H = 0;
    cpu.flags.N = 0;
    cpu.flags.Z = v2 == 0;
    _write8(o, v2);
  }

  // 8-bit Right Rotate
  void RRC(Value8 o) {
    u8 v = _read8(o);
    cpu.flags.Z = v == 0;
    cpu.flags.C = v & 1;
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    _write8(o, (v >> 1) | (v << 7));
  }

  void RLC(Value8 o) {
    u8 v = _read8(o);
    cpu.flags.C = v & 0x80;
    v = (v << 1) | (v >> 7);
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    cpu.flags.Z = v == 0;
    _write8(o, v);
  }

  // 9-bit rotate-left-through-carry
  void RL(Value8 o) {
    u16 v = (_read8(o) << 1) | cpu.flags.C;
    cpu.flags.C = v & 0x100;
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    cpu.flags.Z = (u8)v == 0;
    _write8(o, v);
  }
  
  // 9-bit rotate-right-through-carry
  void RR(Value8 o) {
    u16 v = _read8(o) | (cpu.flags.C << 8);
    cpu.flags.C = v & 1;
    v = v >> 1;
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    cpu.flags.Z = v == 0;
    _write8(o, v);
  }

  void SWAP(Value8 o) {
    u8 value = _read8(o);
    value = value << 4 | (value >> 4);
    _write8(o, value);
    cpu.registers.F = 0;
    cpu.flags.Z = value == 0;
  }
  
  void RST(u8 o) {
    u8 addr = _read8(o);
    _push(*PC_ptr);
    *PC_ptr = addr;
  }

  void CP(Value8 o) {
    u8 a = cpu.registers.A;
    u8 b = _read8(o);
    cpu.flags.Z = a == b;
    cpu.flags.C = a < b;
    cpu.flags.N = 1;
    cpu.flags.H = (a & 0xF) < (b & 0xF);
  }
  void PUSH(Register16 o) {
    _push(_read16(o));
  }
  void POP(Register16 o) {
    _write16(o, _pop());
  }
  
  bool _check(Conditions o) {
    switch(o) {
    case Conditions::T: return true;
    case Conditions::C: return cpu.flags.C;
    case Conditions::NC: return !cpu.flags.C;
    case Conditions::Z: return cpu.flags.Z;
    case Conditions::NZ: return !cpu.flags.Z;
    }
    error = 100;
    log("error condition", o);
    return false;
  }
  
  void RET(Conditions o) {
    if (_check(o)) *PC_ptr = _pop();
  }
  void RETI(Conditions o) {
    // log(*PC_start_ptr, "reti");
    cpu.IME = 1;
    *PC_ptr = _pop();
  }
  void JR(Conditions o, Value8 v) {
    if (_check(o)) *PC_ptr += (i8) _read8(v);
    static bool warned = false;
    if (_check(o) && _read8(v) == 0xfe && !warned)
      log(*PC_start_ptr, "warning: infinite loop", warned = true);
  }
  void JP(Conditions o, Value16 v) {
    if (_check(o))
      *PC_ptr = _read16(v);
  }
  void CALL(Conditions o, Value16 v) {
    u16 addr = _read16(v);
    // if (*PC_start_ptr > 0x100 &&
    //     *PC_start_ptr != 0xc015 &&
    //     addr != 0xc05a)
    //   log(*PC_start_ptr, __FUNCTION__, o, v);
    if (_check(o)) {
      _push(*PC_ptr);
      *PC_ptr = addr;
    }
  }
};
