#pragma once
#include "instructions.hpp"
#include "wasm_host.hpp"

// Registers
using reg8 = u8;
struct reg16 {
  u8 h, l;
  reg16() = default;
  reg16(u16 v) { h = v >> 8; l = v; }
  operator u16 () { return (u16)(h * 0x100 + l); }
};
namespace logs {
  void _log(reg16 v) { _logx16(v.h * 256 + v.l); };
}
using logs::_log;

struct InstructionRunner {
  InstructionRunner() {
    registers.BC = registers.DE =
      registers.AF = registers.HL =
      registers.SP = 0;
  }
  
  struct { 
    union { 
      struct { reg8 B, C, D, E, A, F, H, L; };
      struct { reg16 BC, DE, AF, HL, SP; };
    };
    void dump() {
      log(". . . . . . . . . A  F  B  C  D  E  HL   SP");
      log(". . . . . . . . .", A, F, B, C, D, E, HL, SP);
    }
  } registers;

  struct {
    bool Z = 0, C = 0, N = 0, H = 0;
  } fl;

  u16 *PC_ptr;
  u16 *PC_start_ptr;
  
  int error = 0;

  u8 memory[64 * 1024 ];
  
  u8 _read8_addr(u16 addr) {
    return memory[addr];
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
    case Value8::LdReg8: return _read8_addr(v.addr);
    default: log("read8"); error = 1; return -1;
    }
  }
  void _write8_addr(u16 target, u8 value) {
    memory[target] = value;
    if (0xFF00 <= target  && target < 0xFF80) {
      log("    IO", target, value);
    }
  }
  void _write8_reg(Register8 target, u8 value) {
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
    case Value8::LdDecReg8: {
      u16 addr = _read16(target.reg16);
      _write8_addr(addr, value);
      _write16(target.reg16, addr - 1);
      break;
    }
    case Value8::REG8: {
      _write8_reg(target.reg, value);
      break;
    }
    case Value8::IO_I8: {
      _write8_addr(0xFF00 + target.value, value); break;
    }
    case Value8::IO_R8: {
      _write8_addr(0xFF00 + _read8(target.reg), value); break;
    }
    case Value8::LdReg8: {
      _write8_addr(_read16(target.addr), value); break;
    }
    default:
      log("write8-error", target);
      error = 1; return;
    }
  }
  
  u16 _read16_addr(u16 addr) {
    log("read16-addr");
    error = 1;
    return -1;
  }
  u16 _read16(Register16 r) {
    switch(r) {
    case Register16::BC: return registers.BC; break;
    case Register16::DE: return registers.DE; break;
    case Register16::HL: return registers.HL; break;
    case Register16::SP: return registers.SP; break;
    case Register16::AF: return registers.AF; break;
    default: error = 1; return -1; break;
    }}
  u16 _read16(Value16 v) {
    switch(v.type) {
    case Value16::IMM16: return v.value;
    case Value16::REG16: return _read16(v.reg);
    case Value16::SP_d8: return _read16_addr((u16)registers.SP + (i8)v.offset);
    }
    log("read16");
    error = 1;
    return -1;
  }
  void _write16(Register16 r, u16 value) {
    switch(r) {
    case Register16::BC: registers.BC = value; break;
    case Register16::DE: registers.DE = value; break;
    case Register16::HL: registers.HL = value; break;
    case Register16::SP: registers.SP = value; break;
    default: error = 1;
    }}
  void _write16_addr(u16 addr, u16 value) {
    memory[addr++] = value >> 8;
    memory[addr] = value;
  }
  void _write16(Value16 target, u16 value) {
    switch(target.type) {
    case Value16::REG16: return _write16(target.reg, value);
    case Value16::SP_d8: return _write16_addr((u16)((u16)registers.SP + (i8)target.offset), value);
    default:
      log("write16-error", target);
      error = 1; return;
    }
  }
  
  void NOP() { log(*PC_start_ptr, __FUNCTION__); }
  void STOP() { log(*PC_start_ptr, __FUNCTION__); }
  void DAA() { log(*PC_start_ptr, __FUNCTION__); }

  void CPL() { log(*PC_start_ptr, __FUNCTION__); }
  void SCF() { log(*PC_start_ptr, __FUNCTION__); }
  void CCF() { log(*PC_start_ptr, __FUNCTION__); }

  void DI() { log(*PC_start_ptr, __FUNCTION__); }
  void EI() { log(*PC_start_ptr, __FUNCTION__); }
  void HALT() { log(*PC_start_ptr, __FUNCTION__); }

  void RLCA() { log(*PC_start_ptr, __FUNCTION__); }
  void RRCA() { log(*PC_start_ptr, __FUNCTION__); }
  void RLA() { log(*PC_start_ptr, __FUNCTION__); }
  void RRA() { log(*PC_start_ptr, __FUNCTION__); }
  
  void LD8(Value8 o, Value8 v) {
    error = -1;
    log(*PC_start_ptr, __FUNCTION__, o, v);
    _write8(o, _read8(v));
    // registers.dump();
  }
  void LD16(Value16 o, Value16 v) {
    error = -1;
    log(*PC_start_ptr, __FUNCTION__, o, v);
    _write16(o, _read16(v));
    // registers.dump();
  }

  void BIT(u8 o, Value8 v) {
    if (o < 0 || o > 7) { log("bit", o, v); error = 3; return; }
    // void BIT(Value8 o, Value8 v) {
    error = -1;
    // log(*PC_start_ptr, __FUNCTION__, o, v);
    u8 val = _read8(v) >> o;
    fl.Z = val == 0;
    fl.N = 0;
    fl.H = 1;
  }
  void RES(Value8 o, Value8 v) { log(*PC_start_ptr, __FUNCTION__, o, v); }
  void SET(Value8 o, Value8 v) { log(*PC_start_ptr, __FUNCTION__, o, v); }
  void ADD(Value16 o, Value16 v) { log(*PC_start_ptr, __FUNCTION__, o, v); }
  void ADD(Value8 o, Value8 v) { log(*PC_start_ptr, __FUNCTION__, o, v); }
  // void ADD(Operand o, Operand v) { log(*PC_start_ptr, __FUNCTION__, o, v); }
  void ADC(Value8 o, Value8 v) { log(*PC_start_ptr, __FUNCTION__, o, v); }

  void XOR(Value8 o) {
    error = -1;
    log(*PC_start_ptr, __FUNCTION__, o);
    registers.A ^= _read8(o);
  }
  void AND(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void OR(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void SBC(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void DEC(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); } 
  void DEC(Register16 o) { log(*PC_start_ptr, __FUNCTION__, o); } 
  void INC(Value8 o) {
    error = -1;
    log(*PC_start_ptr, __FUNCTION__, o);
    _write8(o, _read8(o) + 1);
  }  // INC LoadHL)
  void INC(Register16 o) { log(*PC_start_ptr, __FUNCTION__, o); } // INC HL
  void SUB(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  
  void SRL(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void SRA(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void SLA(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void RRC(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void RLC(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void RL(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void RR(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }

  void SWAP(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void RST(u8 o) { log(*PC_start_ptr, __FUNCTION__, o); }

  void CP(Value8 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void PUSH(Register16 o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void POP(Register16 o) { log(*PC_start_ptr, __FUNCTION__, o); }

  void RET(Conditions o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void RETI(Conditions o) { log(*PC_start_ptr, __FUNCTION__, o); }
  void JR(Conditions o, Value8 v) {
    // log(*PC_start_ptr, __FUNCTION__, o, v);
    switch(o) {
    case Conditions::C: if (fl.C) *PC_ptr += (i8) _read8(v); break;
    case Conditions::NZ:
      if (!fl.Z) { *PC_ptr += (i8) _read8(v); }
      break; 
    default:
      error = 100;
    }
    error--;
  }
  void JP(Conditions o, Value16 v) { log(*PC_start_ptr, __FUNCTION__, o, v); }
  void CALL(Conditions o, Value16 v) { log(*PC_start_ptr, __FUNCTION__, o, v); }
};
