#include "base.hpp"
#include "wasm_host.hpp"
#include "instruction_printer.hpp"
#include "instruction_runner.hpp"

struct InstructionDecoder {
  u8 * buf;
  u32 buflen;
  u32 pc, pc_start;
  bool error = 0;

  InstructionPrinter ii;
  // InstructionRunner ii;
  
  InstructionDecoder(u8 * buf, u32 len, u32 pos): buf(buf), buflen(len), pc(pos) { }

  void EOF() { log(__FUNCTION__); error = 1; }

  u8 Imm8() { return buf[pc++]; }
  u8 ImmI8() { return buf[pc++]; }
  u16 Imm16() { u16 v = buf[pc++]; v = v + 256 * buf[pc++]; return v; }


  Value16 Load16(Value16 addr) { return {0}; }
  // Value16 Inc16(Register16 addr);
  Value8 Inc8(Register16 addr) { return {0}; }
  Value8 Dec8(Register16 addr) { return {0}; }
  Value8 Load8(Value16 addr) { return {0}; }
  Value8 IO(Value8 port) {
    Value8 v = port;
    if (port.type == Value8::REG8) v.type = Value8::IO_R8;
    else if (port.type == Value8::IMM8) v.type = Value8::IO_I8;
    else { error = 1; }
    return v;
  }
  // Value8  Load(Register8  addr);
  
  Value16 AddSP(u8 offset) { return Value16::SP_offset(offset); }
  
  // Value16 Load16(Value16 addr) {
  //   log("error in loading", addr);
  //   error = 1;
  //   return {IMM16};8
  // }
  
  // Operand LoadSP(Operand a) {
  //   log("error in loading", a);
  //   error = 1;
  //   return a;
  // }

  // Value16 Load(Register16 a);  

  // // post increment address and get 8-bit value
  // Value8 Dec(Register16 a) {
  //   log("error in Loading ", a);
  //   error = 1;
  //   return {IMM8};
  // }
  // // post increment address and get 8-bit value
  // Value8 Inc(Register16 a) {
  //   log("error in Loading ", a);
  //   error = 1;
  //   return {IMM8};
  // }

  // Operand IO(Operand o) {
  //   if (o.type == OperandType::REG8)
  //     return { OperandType::IO_REG, o.data.val8 };
  //   if (o.type == OperandType::IMM8)
  //     return { OperandType::IO_IMM8, o.data.val8 };
  //   return {(OperandType)-1, {(u8)0}};
  // }
  
  Register16 SP() { return Register16::SP; }
  Register16 BC() { return Register16::BC; }
  Register16 DE() { return Register16::DE; }
  Register16 HL() { return Register16::HL; }
  Register16 AF() { return Register16::AF; }
  Register8 A() { return Register8::A; }
  Register8 B() { return Register8::B; }
  Register8 C() { return Register8::C; }
  Register8 D() { return Register8::D; }
  Register8 E() { return Register8::E; }
  Register8 F() { return Register8::F; }
  Register8 H() { return Register8::H; }
  Register8 L() { return Register8::L; }
  // Operand A() { return {OperandType::REG8, (u8)Register::A }; }
  // Operand B() { return {OperandType::REG8, (u8)Register::B }; }
  // Operand C() { return {OperandType::REG8, (u8)Register::C }; }
  // Operand D() { return {OperandType::REG8, (u8)Register::D }; }
  // Operand E() { return {OperandType::REG8, (u8)Register::E }; }
  // // Operand F() { return {OperandType::REG8, (u8)Register::F }; }
  // Operand H() { return {OperandType::REG8, (u8)Register::H }; }
  // Operand L() { return {OperandType::REG8, (u8)Register::L }; }

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
