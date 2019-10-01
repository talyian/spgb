#pragma once
#include "instructions.hpp"
#include "wasm_host.hpp"
#include "memory_mapper.hpp"
#include "cpu.hpp"

  
struct InstructionRunner {
  InstructionRunner(CPU &cpu) :
    cpu(cpu),
    registers(cpu.registers),
    fl(cpu.flags) {  }
  
  CPU &cpu;
  Registers &registers;
  Flags &fl;
  
  u16 *PC_ptr;
  u16 *PC_start_ptr;
  
  int error = 0;
  int verbose_log = 0;
  MemoryMapper *mmu = 0;

  void dump() {
    log(". . . . . . . . . A  F  B  C  D  E  HL   SP");
    log(". . . . . . . . .", registers.A, registers.F,
        registers.B, registers.C,
        registers.D, registers.E,
        registers.HL, registers.SP);
  }
  
  template<class T, class ...TS>
  void m_log(T x, TS ... xs) {
    // if (verbose_log || error)
    //   log(x, xs...);
  }
  
  void _push(reg16 value) {
    mmu->set(--registers.SP, value.h);
    mmu->set(--registers.SP, value.l);
  }
  u16 _pop() {
    u16 l = mmu->get(registers.SP++);
    u16 h = mmu->get(registers.SP++);
    return h * 0x100 + l;
  }
  
  u8 _read8_addr(u16 addr) {
    // if (0xFF00 <= addr && addr < 0xFF80 && addr != 0xFF44 && addr != 0xFF42)
    //   log("    IO:", addr);
    return mmu->get(addr);
  }
  u8 _read8(Register8 r) {
    switch(r) {
#define X(RR)     case Register8::RR: return registers.RR; break;
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
    if (0xFF00 <= addr) _handle_io_write(addr, value);
    mmu->set(addr, value);
  }
  void _write8(Register8 target, u8 value) {
    switch(target) {
#define X(RR)     case Register8::RR: registers.RR = value; break;
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
    return value * 0x100 + mmu->get(addr);
  }
  u16 _read16(Register16 r) {
    switch(r) {
    case Register16::BC: return registers.BC; break;
    case Register16::DE: return registers.DE; break;
    case Register16::HL: return registers.HL; break;
    case Register16::SP: return registers.SP; break;
    case Register16::AF: return registers.AF; break;
    }
  }
  u16 _read16(Value16 v) {
    switch(v.type) {
    case Value16::IMM16: return v.value;
    case Value16::REG16: return _read16(v.reg);
    case Value16::SP_d8: return _read16_addr((u16)registers.SP + (i8)v.offset);
    }
  }
  void _write16(Register16 r, u16 value) {
    switch(r) {
    case Register16::BC: registers.BC = value; break;
    case Register16::DE: registers.DE = value; break;
    case Register16::HL: registers.HL = value; break;
    case Register16::SP: registers.SP = value; break;
    case Register16::AF: registers.AF = value & 0xFFF0; break;
    }
  }
  void _write16_addr(u16 addr, u16 value) {
    mmu->set(addr++, value >> 8);
    mmu->set(addr, value);
  }
  void _write16(Value16 target, u16 value) {
    switch(target.type) {
    case Value16::IMM16: log("err-write-to-imm16"); return;
    case Value16::REG16: return _write16(target.reg, value);
    case Value16::SP_d8:
      return _write16_addr((u16)((u16)registers.SP + (i8)target.offset), value);
    }
  }

  void _handle_io_write(u16 addr, u16 value) {
    switch(addr) {
    case 0xFF50: mmu->bios_active = false; break;
    // case 0xFF46: log(*PC_start_ptr, "    (DMA) IO:", addr, "<-", value); break;
    // case 0xFFFF: log(*PC_start_ptr, "     (IE) IO:", addr, "<-", value); break;
    // case 0xFF0F: log(*PC_start_ptr, "     (IF) IO:", addr, "<-", value); break;
    }
  }

  void NOP() { m_log(*PC_start_ptr, __FUNCTION__); }
  void STOP() {
    m_log(*PC_start_ptr, __FUNCTION__);
    cpu.stopped = 1;
    error = 5;
  }
  void DAA() { m_log(*PC_start_ptr, __FUNCTION__);     error = 5;
}

  // Complement A
  void CPL() {
    m_log(*PC_start_ptr, __FUNCTION__);
    registers.A = ~registers.A;
    fl.N = 1;
    fl.H = 1;
  }

  // Set Carry Flag
  void SCF() {
    m_log(*PC_start_ptr, __FUNCTION__);
    fl.N = 0;
    fl.H = 0;
    fl.C = 1;
  }

  // Complement Carry Flag
  void CCF() {
    m_log(*PC_start_ptr, __FUNCTION__);
    fl.N = 0;
    fl.H = 0;
    fl.C = ~fl.C;
  }

  // Disable Interrupts
  void DI() {
    m_log(*PC_start_ptr, __FUNCTION__);
    cpu.IME = 0;
  }

  // Enable Interrupts
  void EI() {
    m_log(*PC_start_ptr, __FUNCTION__);
    cpu.IME = 1;
  }

  void HALT() {
    m_log(*PC_start_ptr, __FUNCTION__);
    cpu.halted = 1;
  }

  void RLCA() {
    m_log(*PC_start_ptr, __FUNCTION__);
    return RLC(Register8::A);
  }
  void RRCA() {
    m_log(*PC_start_ptr, __FUNCTION__);
    return RRC(Register8::A);
  }
  void RLA() {
    return RL(Register8::A);
  }
  void RRA() {
    m_log(*PC_start_ptr, __FUNCTION__);
    return RR(Register8::A);
  }
  
  void LD8(Value8 o, Value8 v) {
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    _write8(o, _read8(v));
  }
  void LD16(Value16 o, Value16 v) {
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    _write16(o, _read16(v));
  }

  void BIT(u8 o, Value8 v) {
    if (o < 0 || o > 7) { log("bit", o, v); error = 3; return; }
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    u8 val = _read8(v) >> o;
    fl.Z = val == 0;
    fl.N = 0;
    fl.H = 1;
  }

  void RES(u8 o, Value8 v) {
    if (o < 0 || o > 7) { log("bit", o, v); error = 3; return; }
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    u8 val = _read8(v) & ~(1 << 0);
    _write8(v, val);
  }
  void SET(u8 o, Value8 v) {
    if (o < 0 || o > 7) { log("bit", o, v); error = 3; return; }
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    u8 val = _read8(v) | (1 << 0);
    _write8(v, val);
  }
  void ADD(Value16 o, Value16 v) {
    // TODO: addSP has different flags and timing!
    m_log(*PC_start_ptr, __FUNCTION__, o, v); 
    u16 a = _read16(o);
    u16 b = _read16(v);
    _write16(o, a + b);
    fl.N = 0;
    fl.C = a > ~b;
    fl.H = (a & 0xFFF) > ~(b & 0xFFF);
  }

  void ADD(Value8 o, Value8 v) {
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    u8 a = _read8(o);
    u8 b = _read8(v);
    _write8(o, a + b);
    fl.Z = (u8)(a + b) == 0;
    fl.C = (u16)a + (u16)b > 0xFF;
    fl.H = (a & 0xF) + (b & 0xF) > 0xF;
    fl.N = 0;
  }

  void ADC(Value8 o, Value8 v) {
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    u8 a = _read8(o);
    u8 b = _read8(v);
    u16 d = a + b + fl.C;
    _write8(o, d);
    fl.Z = (u8)d == 0;
    fl.C = d > 0xFF;
    fl.H = (a & 0xF) + (b & 0xF) + fl.C > 0xF;
    fl.N = 0;
  }

  void XOR(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    registers.A = registers.A ^ _read8(o);
    fl.N = 0;
    fl.H = 0;
    fl.C = 0;
    fl.Z = registers.A = 0;
  }
  void AND(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    registers.A = registers.A & _read8(o);
    fl.N = 0;
    fl.H = 1;
    fl.C = 0;
    fl.Z = registers.A = 0;
  }
  void OR(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    registers.A = registers.A | _read8(o);
    fl.N = 0;
    fl.H = 0;
    fl.C = 0;
    fl.Z = registers.A = 0;
  }

  void DEC(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 a = _read8(o);
    u8 v = a - (u8)1;
    _write8(o, v);
    fl.Z = a == 1;
    fl.N = 1;
    fl.H = (a & 0xF) < 1;
  } 
  void DEC(Register16 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u16 v = _read16(o);
    _write16(o, v - 1);
    // no flags
  }
  
  void INC(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 v = _read8(o);
    _write8(o, v + 1);
    fl.N = 0;
    fl.H = (v & 0xF) == 0xF;
    fl.Z = v == 0xFF;
  }  // INC LoadHL)

  void INC(Register16 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    _write16(o, _read16(o) + 1);
    // no flags
  } // INC HL

  void SUB(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 a = _read8(Register8::A);
    u8 b = _read8(o);
    _write8(Register8::A, a - b);
    fl.Z = a == b;
    fl.N = 1;
    fl.C = a < b;
    fl.H = (a & 0xF) < (b & 0xF);
  }

  void SBC(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 a = _read8(Register8::A);
    u8 b = _read8(o);
    u8 c = a - b - fl.C;
    _write8(Register8::A, c);
    fl.Z = a == b + fl.C;
    fl.N = 1;
    fl.C = a < b + fl.C;
    fl.H = (a & 0xF) < ((b + fl.C) & 0xF);
    
  }

  // unsigned right-shift
  void SRL(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 v = _read8(o);
    fl.C = v & 1;
    fl.N = 0;
    fl.H = 0;
    fl.Z = (v >> 1) == 0;
    _write8(o, v >> 1);
  }

  // signed right-shift
  void SRA(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 v = _read8(o);
    fl.C = v & 1;
    fl.H = 0;
    fl.N = 0;
    fl.Z = (((i8)v) >> 1) == 0;
    _write8(o, ((i8)v) >> 1);
  }

  // left shift
  void SLA(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 v = _read8(o);
    u8 v2 = v << 1;
    fl.C = v & 0x80;
    fl.H = 0;
    fl.N = 0;
    fl.Z = v2 == 0;
    _write8(o, v2);
  }

  // 8-bit Right Rotate
  void RRC(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 v = _read8(o);
    fl.Z = v == 0;
    fl.C = v & 1;
    fl.N = 0;
    fl.H = 0;
    _write8(o, (v >> 1) | (v << 7));
  }

  void RLC(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 v = _read8(o);
    fl.C = v & 0x80;
    v = (v << 1) | (v >> 7);
    fl.N = 0;
    fl.H = 0;
    fl.Z = v == 0;
    _write8(o, v);
  }

  // 9-bit rotate-left-through-carry
  void RL(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u16 v = (_read8(o) << 1) | fl.C;
    fl.C = (v >> 8) & 1;
    fl.N = 0;
    fl.H = 0;
    fl.Z = v == 0;
    _write8(o, v);
  }
  
  // 9-bit rotate-right-through-carry
  void RR(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u16 v = _read8(o) | (fl.C << 8);
    fl.C = v & 1;
    v = v >> 1;
    fl.N = 0;
    fl.H = 0;
    fl.Z = v == 0;
    _write8(o, v);
  }

  void SWAP(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 value = _read8(o);
    value = value << 4 | (value >> 4);
    _write8(o, value);
    registers.F = 0;
    fl.Z = value == 0;
  }
  
  void RST(u8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    _push(*PC_ptr);
    *PC_ptr = _read8(o);
  }

  void CP(Value8 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    u8 a = registers.A;
    u8 b = _read8(o);
    fl.Z = a == b;
    fl.C = a < b;
    fl.N = 0;
    fl.H = (a & 0xF) < (b & 0xF);
  }
  void PUSH(Register16 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    _push(_read16(o));
  }
  void POP(Register16 o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    _write16(o, _pop());
  }
  
  bool _check(Conditions o) {
    switch(o) {
    case Conditions::T: return true;
    case Conditions::C: return fl.C;
    case Conditions::NC: return !fl.C;
    case Conditions::Z: return fl.Z;
    case Conditions::NZ: return !fl.Z;
    }
    error = 100;
  }
  
  void RET(Conditions o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    if (_check(o)) *PC_ptr = _pop();
  }
  void RETI(Conditions o) {
    m_log(*PC_start_ptr, __FUNCTION__, o);
    log(*PC_start_ptr, "reti");
    cpu.IME = 1;
    *PC_ptr = _pop();
  }
  void JR(Conditions o, Value8 v) {
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    if (_check(o)) *PC_ptr += (i8) _read8(v);
    static bool warned = false;
    if (_check(o) && _read8(v) == 0xfe && !warned)
      log(*PC_start_ptr, "warning: infinite loop", warned = true);
  }
  void JP(Conditions o, Value16 v) {
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    if (_check(o))
      *PC_ptr = _read16(v);
  }
  void CALL(Conditions o, Value16 v) {
    m_log(*PC_start_ptr, __FUNCTION__, o, v);
    _push(*PC_ptr);
    *PC_ptr = _read16(v);
  }
};
