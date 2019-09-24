#include "base.hpp"
#include "wasm_host.hpp"
#include "instruction_printer.hpp"
#include "instruction_runner.hpp"

struct InstructionDecoder {
  u8 * buf;
  u32 buflen;
  u32 pc, pc_start;
  bool error = 0;

  // InstructionPrinter ii;
  InstructionRunner ii;
  
  InstructionDecoder(u8 * buf, u32 len, u32 pos): buf(buf), buflen(len), pc(pos) { }

  void EOF() { log(__FUNCTION__); error = 1; }

  Operand Imm16() {
    u16 v = buf[pc++]; v = v + 256 * buf[pc++];
    return {OperandType::IMM16, {v}};
  }

  Operand Imm8() { return {OperandType::IMM8, {buf[pc++]}}; }

  Operand ImmI8() { return {OperandType::IMM8, {buf[pc++]}}; } // TODO

  Operand Load16(Operand a) {
    log("error in loading", a);
    error = 1;
    return a;
  }
  
  Operand LoadSP(Operand a) {
    log("error in loading", a);
    error = 1;
    return a;
  }
  
  Operand Load(Operand a) {
    if (a.type == OperandType::REG16)
      return {OperandType::Load_REG16, {a.data.val8}};
    if (a.type == OperandType::IMM16)
      return {OperandType::Load_IMM16, {a.data.val16}};
    else 
    log("error in Loading ", a);
    error = 1;
    return {OperandType::IMM8, {(u8)0xFF}};
  }

  Operand Dec(Operand a) {
    if (a.type == OperandType::REG16)
      return {OperandType::Dec_REG16, {a.data.val8}};
    log("error in Loading ", a);
    error = 1;
    return {OperandType::IMM8, {(u8)0xFF}};
  }

  Operand Inc(Operand a) {
    if (a.type == OperandType::REG16)
      return {OperandType::Inc_REG16, {a.data.val8}};
    log("error in Loading ", a);
    error = 1;
    return {OperandType::IMM8, {(u8)0xFF}};
  }

  Operand IO(Operand o) {
    if (o.type == OperandType::REG8)
      return { OperandType::IO_REG, o.data.val8 };
    if (o.type == OperandType::IMM8)
      return { OperandType::IO_IMM8, o.data.val8 };
    return {(OperandType)-1, {(u8)0}};
  }
  
  Operand SP() { return {OperandType::REG16, (u8)Register::SP }; }
  Operand BC() { return {OperandType::REG16, (u8)Register::BC }; }
  Operand DE() { return {OperandType::REG16, (u8)Register::DE }; }
  Operand HL() { return {OperandType::REG16, (u8)Register::HL }; }
  Operand AF() { return {OperandType::REG16, (u8)Register::AF }; }
  Operand A() { return {OperandType::REG8, (u8)Register::A }; }
  Operand B() { return {OperandType::REG8, (u8)Register::B }; }
  Operand C() { return {OperandType::REG8, (u8)Register::C }; }
  Operand D() { return {OperandType::REG8, (u8)Register::D }; }
  Operand E() { return {OperandType::REG8, (u8)Register::E }; }
  // Operand F() { return {OperandType::REG8, (u8)Register::F }; }
  Operand H() { return {OperandType::REG8, (u8)Register::H }; }
  Operand L() { return {OperandType::REG8, (u8)Register::L }; }

  Conditions CC() { return Conditions::C; }
  Conditions CZ() { return Conditions::Z; }
  Conditions CT() { return Conditions::T; }
  Conditions CNC() { return Conditions::NC; }
  Conditions CNZ() { return Conditions::NZ; }
  
  void decode() {
    if (pc >= buflen) return EOF();
    _log((u16)pc);
    pc_start = pc;
    u16 op = buf[pc++];
    if (op == 0xcb) op = 0x100 + buf[pc++];
    switch(op) {
    #include "generated_instruction_decode_table.inc"
    default:
      log("unknown op", op);
      error = 1;
    }
  }
};
