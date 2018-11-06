#pragma once
#include <cstdio>


#include "registers.hh"
#include "memory.hh"
#include "io.hh"

#include "instructions.hh"

struct OpPrint {
  Registers &reg;
  OpPrint(Registers &reg) : reg(reg) { }

  void NOP() { pp("NOP"); }
  void HALT() { pp("HALT"); }

  void LD(Val8 dst, Val8 src) { pp("LD", dst, src); }
  void LD(Val16 dst, Val16 src) { pp("LD", dst, src); }

  // stack operations
  void PUSH(Val16 val) { pp("PUSH", val); }
  void POP(Val16 addr) { pp("POP", addr); }

  // jumps
  void JP(Cond cond, Val16 dst) { pp("JP", cond, dst); }
  void JR(Cond cond, Val8 offset) { pp("JR", cond, offset); }
  void RET(Cond cond) { pp("RET", cond); }
  void CALL(Cond cond, Val16 target) { pp("CALL", cond, target); }
  void RST(u8 val) { pp("RST", val); }
  // tests
  void CP(Val8 val)   { pp("CP", val); }
  void INC(Val8 dst)  { pp("INC", dst); }
  void INC(Val16 dst) { pp("INC", dst); }
  void DEC(Val8 dst)  { pp("DEC", dst); }
  void DEC(Val16 dst) { pp("DEC", dst); }

  // bitwise
  void RR(Val8 val)   { pp("RR", val); }
  void RRC(Val8 val)  { pp("RRC", val); }
  void RL(Val8 val)   { pp("RL", val); }
  void RLC(Val8 val)  { pp("RLC", val); }
  void SLA(Val8 val)  { pp("SLA", val); }
  void SRA(Val8 val)  { pp("SRA", val); }
  void SRL(Val8 val)  { pp("SRL", val); }
  void SWAP(Val8 val) { pp("SWAP", val); }

  // single-bit
  void RES(int bit, Val8 val) { pp("RES", bit, val); }
  void SET(int bit, Val8 val) { pp("SET", bit, val); }
  void BIT(int bit, Val8 val) { pp("BIT", bit, val); }

  // Arithmetic
  void XOR(Val8 val) { pp("XOR", val); }
  void AND(Val8 val) { pp("AND", val); }
  void OR(Val8 val) { pp("OR", val); }
  void ADD(Val8 dst, Val8 val) { pp("ADD", dst, val); }
  void ADC(Val8 dst, Val8 val) { pp("ADC", dst, val); }
  void ADD(Val16 dst, Val16 val) { pp("ADD", dst, val); }
  void SUB(Val8 val) { pp("SUB", val); }
  void SBC(Val8 dst, Val8 val) { pp("SBC", dst, val); }

  void DI() { pp("DI"); }
  void EI() { pp("EI"); }

  void p(const char *s) { printf("%s", s); }
  void p(int x) { printf("%x", x); }
  void p(Val8 v) { show(v); }
  void p(Val16 v) { show(v); }
  void p(Cond c) { show(c); }

  template<class T>
  void _pp(T single) { p(single); p("\n"); }
  template<class T, class ...TS>
  void _pp(T first, TS ...rest) {
    p(first);
    p(" ");
    _pp(rest...);
  }
  u16 pc = 0;
  template<class ...TS>
  void pp(TS ...rest) {
    printf("[%04x] ", pc);
    _pp(rest...);
    pc = reg.PC;
  }
};

struct Executor {
  Memory &mem;
  Registers &reg;
  uint64_t timer = 0;
  Executor(Registers &reg, Memory &memory) : mem(memory), reg(reg) { }

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
};
