// Instructions - data types for encoding an instruction and its operands
#pragma once
#include "registers.hh"
#include "memory.hh"

#include <cstdint>
typedef uint8_t u8;
typedef uint16_t u16;

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

void show(Cond c);
void show(Val16 v);
void show(Val8 v);
