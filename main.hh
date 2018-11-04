#include "registers.hh"
#include "memory.hh"

// Register Types
enum class REG8 { A, B, C, D, E, F, H, L };
enum class REG16 { BC, DE, HL, AF, SP, PC };
struct LoadType { } LOAD;
// X macros for looping over all 8-bit and 16-bit registers
#define FORALL_REG8(SS) SS(A) SS(B) SS(C) SS(D) SS(E) SS(F) SS(H) SS(L)
#define FORALL_REG16(SS) SS(BC) SS(DE) SS(HL) SS(AF) // SS(SP) SS(PC)

// An 8-bit operand can be any of:
//  - an immediate 8-bit value
//  - an 8-bit register
//  - 16-bit value as a pointer
//  - 8-bit IO port
struct Val8 {
  enum { Reg, Val, PtrN, PtrR, IoN, IoR } type;
  union {
    REG8 r;
    u8 value;
    u16 ptr_n;
    REG16 ptr_r;
    u8 io_n;
    REG8 io_r;
  } value;

  u8 get(Registers &reg, Memory &mem);
  void set(Registers &reg, Memory &mem, Val8 source);
  Val8(u8 val) { type = Val; value.value = val; }
  Val8(REG8 r) { type = Reg; value.r = r; }
  Val8(LoadType _, REG16 addr) { type = PtrR; value.ptr_r = addr; }
  Val8(LoadType _, u16 addr) { type = PtrN; value.ptr_n = addr; }
};

// A 16-bit operand can be any of the following:
//  - a 16-bit register
//  - a 16-bit immediate value
struct Val16 {
  enum { Reg, Val } type;
  union {
    REG16 r;
    u16 value;
  } value;
  u16 get(Registers &reg, Memory &mem);
  void set(Registers &reg, Memory &mem, Val16 source);
  Val16(u16 val) { type = Val; value.value=val; }
  Val16(REG16 r) { type = Reg; value.r = r; }
};

enum class Cond { ALWAYS, Z, NZ, C, NC };

struct Executor {
  Memory &mem;
  Registers &reg;

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

  // jumps
  void JP(Cond cond, Val16 dst);
  void JR(Cond cond, Val8 offset);
  void RET(Cond cond);
  void CALL(Cond cond, Val16 target);

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
  void SWAP(Val8 val);

  // single-bit
  void RES(int bit, Val8 val);
  void SET(int bit, Val8 val);
  void BIT(int bit, Val8 val);

  // Arithmetic
  void XOR(Val8 val);
  void ADD(Val8 val);
  void SUB(Val8 val);
};
