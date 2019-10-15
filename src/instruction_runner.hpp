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

  void dump();
  
  template<class T, class ...TS>
  void m_log(T x, TS ... xs) {
    // if (verbose_log || error)
    //   log(x, xs...);
  }
  
  void _push(Reg16 value);
  u16 _pop();
  
  u8 _read8_addr(u16 addr);
  u8 _read8(Register8 r);
  u8 _read8(Value8 v);
  void _write8_addr(u16 addr, u8 value);
  void _write8(Register8 target, u8 value);
  void _write8(Value8 target, u8 value);
  
  u16 _read16_addr(u16 addr);
  u16 _read16(Register16 r);
  u16 _read16(Value16 v);
  void _write16(Register16 r, u16 value);
  void _write16_addr(u16 addr, u16 value);
  void _write16(Value16 target, u16 value);

  // case 0xFF02:
  // if (value & 0x80) {
  //   cpu.serial.out_buf[cpu.serial.pos++] = mmu->get(0xFF01);
  //   mmu->set(0xFF02, value & ~0x80);
  // }
  // break;

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
    u8 v2 = ((i8)v) >> 1;
    cpu.flags.C = v & 1;
    cpu.flags.H = 0;
    cpu.flags.N = 0;
    cpu.flags.Z = v2 == 0;
    _write8(o, v2);
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
    u8 v2 = (v >> 1) | (v << 7);
    cpu.flags.Z = v == 0;
    cpu.flags.C = v & 1;
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    _write8(o, v2);
  }

  void RLC(Value8 o) {
    u8 v = _read8(o);
    cpu.flags.C = v & 0x80;
    u8 v2 = (v << 1) | (v >> 7);
    cpu.flags.N = 0;
    cpu.flags.H = 0;
    cpu.flags.Z = v2 == 0;
    _write8(o, v2);
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
    if (_check(o)) {
      _push(*PC_ptr);
      *PC_ptr = addr;
    }
  }
};
