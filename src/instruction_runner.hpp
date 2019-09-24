#pragma once
#include "instructions.hpp"
#include "wasm_host.hpp"

// Registers
using reg8 = u8;
struct reg16 {
  u8 h, l;
  reg16() = default;
  reg16(u16 v) { h = v >> 8; l = v; }
};
namespace logs {
  void _log(reg16 v) { _logx16(v.h * 256 + v.l); };
}
using logs::_log;

struct InstructionRunner {
  InstructionRunner() {
    registers.A = registers.B =
      registers.C = registers.D = registers.E =
      registers.F = registers.H = registers.L = 0;
  }
  
  struct { 
    union { 
      struct { reg8 B, C, D, E, A, F, H, L; };
      struct { reg16 BC, DE, AF, HL, SP; };
    };

    void dump() {
      log(". . . . . . . . .", A, F, B, C, D, E, HL, SP);
    }
  } registers;
  
  void set_16(Operand o, u16 value) {
    switch(o.type) {
    case OperandType::REG16 :
      switch((Register)o.data.val8) {
      case Register::SP: registers.SP = value; return;
      case Register::HL: registers.HL = value; return;
      default: log("unknown operand register", o); error = 1; break;
      }
    default: log("unknown operand type", o); error = 1; break;
    }
  }
  
  u16 get_16(Operand v) {
    switch(v.type) {
    case OperandType::IMM16: return v.data.val16;
    default:
      error = 1; log("get_16 error", v); return 1;
    }
  }
  bool error = 0;
  
  void NOP() { log(__FUNCTION__); }
  void STOP() { log(__FUNCTION__); }
  void DAA() { log(__FUNCTION__); }

  void CPL() { log(__FUNCTION__); }
  void SCF() { log(__FUNCTION__); }
  void CCF() { log(__FUNCTION__); }

  void DI() { log(__FUNCTION__); }
  void EI() { log(__FUNCTION__); }
  void HALT() { log(__FUNCTION__); }

  void RLCA() { log(__FUNCTION__); }
  void RRCA() { log(__FUNCTION__); }
  void RLA() { log(__FUNCTION__); }
  void RRA() { log(__FUNCTION__); }

  void LD16(Operand o, Operand v) {
    u16 value = get_16(v);
    set_16(o, value);
    registers.dump();
  }
  void LD8(Operand o, Operand v) {
    
  }
  void LD(Operand o, Operand v) {
    log(__FUNCTION__, o, v);
    if (o.type == OperandType::REG16)
      return LD16(o, v);
    error = 1;
  }
  
  void BIT(Operand o, Operand v) { log(__FUNCTION__, o, v); }
  void RES(Operand o, Operand v) { log(__FUNCTION__, o, v); }
  void SET(Operand o, Operand v) { log(__FUNCTION__, o, v); }
  void ADD(Operand o, Operand v) { log(__FUNCTION__, o, v); }
  void ADC(Operand o, Operand v) { log(__FUNCTION__, o, v); }

  void XOR(Operand o) { log(__FUNCTION__, o); }
  void AND(Operand o) { log(__FUNCTION__, o); }
  void OR(Operand o) { log(__FUNCTION__, o); }
  void SBC(Operand o) { log(__FUNCTION__, o); }
  void DEC(Operand o) { log(__FUNCTION__, o); }
  void INC(Operand o) { log(__FUNCTION__, o); }
  void SUB(Operand o) { log(__FUNCTION__, o); }
  
  void SRL(Operand o) { log(__FUNCTION__, o); }
  void SRA(Operand o) { log(__FUNCTION__, o); }
  void SLA(Operand o) { log(__FUNCTION__, o); }
  void RRC(Operand o) { log(__FUNCTION__, o); }
  void RLC(Operand o) { log(__FUNCTION__, o); }
  void SWAP(Operand o) { log(__FUNCTION__, o); }
  void RST(Operand o) { log(__FUNCTION__, o); }

  void CP(Operand o) { log(__FUNCTION__, o); }
  void PUSH(Operand o) { log(__FUNCTION__, o); }
  void POP(Operand o) { log(__FUNCTION__, o); }
  void RL(Operand o) { log(__FUNCTION__, o); }
  void RR(Operand o) { log(__FUNCTION__, o); }

  void RET(Conditions o) { log(__FUNCTION__, o); }
  void RETI(Conditions o) { log(__FUNCTION__, o); }
  void JR(Conditions o, Operand v) { log(__FUNCTION__, o, v); }
  void JP(Conditions o, Operand v) { log(__FUNCTION__, o, v); }
  void CALL(Conditions o, Operand v) { log(__FUNCTION__, o, v); }
};
