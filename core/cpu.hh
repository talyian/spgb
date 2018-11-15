#pragma once
#include "main.hh"

struct CPU {
  Memory &mem;
  Registers &reg;

  uint64_t timer = 0;
  bool halted = false;

  CPU(Registers &reg, Memory &memory) : mem(memory), reg(reg) { }

  bool cond_eval(Cond c) {
    switch(c) {
    case Cond::ALWAYS: return true;
    case Cond::Z: return reg.FZ();
    case Cond::NZ: return !reg.FZ();
    case Cond::C: return  reg.FC();
    case Cond::NC: return !reg.FC();
    }
  }

  u8 get(Val8 a);
  u16 get(Val16 a);
  void set(Val8 a, Val8 v);
  void set(Val16 a, Val16 v);

  void NOP();
  void HALT();
  void LD(Val8 dst, Val8 src);
  void LD(Val16 dst, Val16 src);
  // stack operations
  void PUSH(Val16 val);
  void POP(Val16 addr);
  u16 PEEK();
  // jumps
  void JP(Cond cond, Val16 dst);
  void JR(Cond cond, Val8 offset);
  void RET(Cond cond);
  void CALL(Cond cond, Val16 target);
  void RST(u8 val);
  // tests
  void CP(Val8 val);
  void INC(Val8 dst);
  void INC(Val16 dst);
  void DEC(Val8 dst);
  void DEC(Val16 dst);
  // bitwise
  void RR(Val8 val);
  void RRC(Val8 val);
  void RL(Val8 val);
  void RLC(Val8 val);
  void SLA(Val8 val);
  void SRA(Val8 val);
  void SRL(Val8 val);
  void SWAP(Val8 val);
  // single-bit
  void RES(int bit, Val8 val);
  void SET(int bit, Val8 val);
  void BIT(int bit, Val8 val);
  // Arithmetic
  void XOR(Val8 val);
  void AND(Val8 val);
  void OR(Val8 val);
  void CPL();
  void ADD(Val8 dst, Val8 val);
  void ADC(Val8 dst, Val8 val);
  void ADD(Val16 dst, Val16 val);
  void SUB(Val8 val);
  void SBC(Val8 val);
  // Interrupts
  void DI();
  void EI();
  void STOP();
  void DAA();
  void CCF();
  void SCF();
};
