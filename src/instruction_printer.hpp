#pragma once
#include "instructions.hpp"
#include "platform.hh"

struct InstructionPrinter {
  u16 internal_pc = 0;
  u16 internal_pc_start = 0;

  u16 * PC_ptr = 0;
  u16 * PC_start_ptr = 0;
  int error = 0;
  
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

  void LD8(Value8 o, Value8 v) { log(__FUNCTION__, o, v); }
  void LD16(Value16 o, Value16 v) { log(__FUNCTION__, o, v); }
  
  void BIT(Value8 o, Value8 v) { log(__FUNCTION__, o, v); }
  void RES(Value8 o, Value8 v) { log(__FUNCTION__, o, v); }
  void SET(Value8 o, Value8 v) { log(__FUNCTION__, o, v); }
  void ADD(Value8 o, Value8 v) { log(__FUNCTION__, o, v); }
  void ADD(Value16 o, Value16 v) { log(__FUNCTION__, o, v); }
  void ADC(Value8 o, Value8 v) { log(__FUNCTION__, o, v); }

  void XOR(Value8 o) { log(__FUNCTION__, o); }
  void AND(Value8 o) { log(__FUNCTION__, o); }
  void OR(Value8 o) { log(__FUNCTION__, o); }
  void SBC(Value8 o) { log(__FUNCTION__, o); }
  void DEC(Value8 o) { log(__FUNCTION__, o); } 
  void DEC(Register16 o) { log(__FUNCTION__, o); } 
  void INC(Value8 o) { log(__FUNCTION__, o); }
  void INC(Register16 o) { log(__FUNCTION__, o); }
  void SUB(Value8 o) { log(__FUNCTION__, o); }
  
  void SRL(Value8 o) { log(__FUNCTION__, o); }
  void SRA(Value8 o) { log(__FUNCTION__, o); }
  void SLA(Value8 o) { log(__FUNCTION__, o); }
  void RRC(Value8 o) { log(__FUNCTION__, o); }
  void RLC(Value8 o) { log(__FUNCTION__, o); }
  void RL(Value8 o) { log(__FUNCTION__, o); }
  void RR(Value8 o) { log(__FUNCTION__, o); }

  void SWAP(Value8 o) { log(__FUNCTION__, o); }
  void RST(u8 o) { log(__FUNCTION__, o); }

  void CP(Value8 o) { log(__FUNCTION__, o); }
  void PUSH(Register16 o) { log(__FUNCTION__, o); }
  void POP(Register16 o) { log(__FUNCTION__, o); }

  void RET(Conditions o) { log(__FUNCTION__, o); }
  void RETI(Conditions o) { log(__FUNCTION__, o); }
  void JR(Conditions o, Value8 v) { log(__FUNCTION__, o, v); }
  void JP(Conditions o, Value16 v) { log(__FUNCTION__, o, v); }
  void CALL(Conditions o, Value16 v) { log(__FUNCTION__, o, v); }

};
