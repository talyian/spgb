#pragma once
#include "instructions.hpp"
#include "platform.hpp"
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

  bool _check(Conditions o);
  
  void NOP();  
  void STOP();
  void DAA();

  void CPL();   // Complement A
  void SCF();   // Set Carry Flag
  void CCF();   // Complement Carry Flag
  void DI();
  void EI();
  void HALT();
  void RLCA();
  void RRCA();
  void RLA();
  void RRA();
  void LD8(Value8 o, Value8 v);
  void LD16(Value16 o, Value16 v);

  void BIT(u8 o, Value8 v);
  void RES(u8 o, Value8 v);
  void SET(u8 o, Value8 v);
  
  void ADD(Value16 o, Value16 v);
  void ADD(Value8 o, Value8 v);
  void ADC(Value8 o, Value8 v);
  void XOR(Value8 o);
  void AND(Value8 o);
  void OR(Value8 o);
  void DEC(Value8 o); 
  void DEC(Register16 o);
  void INC(Value8 o);
  void INC(Register16 o);
  void SUB(Value8 o);
  void SBC(Value8 o);
  // unsigned right-shift
  void SRL(Value8 o);
  // signed right-shift
  void SRA(Value8 o);
  // left shift
  void SLA(Value8 o);
  // 8-bit Right Rotate
  void RRC(Value8 o);
  void RLC(Value8 o);
  // 9-bit rotate-left-through-carry
  void RL(Value8 o);
  // 9-bit rotate-right-through-carry
  void RR(Value8 o);

  void SWAP(Value8 o);
  void RST(u8 o);
  void CP(Value8 o);
  void PUSH(Register16 o);
  void POP(Register16 o);

  void RET(Conditions o);
  void RETI(Conditions o);
  void JR(Conditions o, Value8 v) ;
  void JP(Conditions o, Value16 v);
  void CALL(Conditions o, Value16 v);
};
