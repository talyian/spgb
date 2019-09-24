#pragma once
#include "instructions.hpp"
#include "wasm_host.hpp"

struct InstructionPrinter {
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
  
  void LD(Operand o, Operand v) { log(__FUNCTION__, o, v); }
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
