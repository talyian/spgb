#include "base.hpp"
#include "instruction_decoder.hpp"

u8 InstructionDecoder::Imm8() { return mmu->get(pc++); }
u8 InstructionDecoder::ImmI8() { return mmu->get(pc++); }
u16 InstructionDecoder::Imm16() { u16 v = mmu->get(pc++); v = v + 256 * mmu->get(pc++); return v; }

Value16 InstructionDecoder::Load16(Value16 addr) { return {0}; }
  // Value16 Inc16(Register16 addr);
Value8 InstructionDecoder::Inc8(Register16 addr) {
    Value8 v;
    v.type = Value8::LdIncReg8;
    v.reg16 = addr;
    return v;
  }
Value8 InstructionDecoder::Dec8(Register16 addr) {
    Value8 v;
    v.type = Value8::LdDecReg8;
    v.reg16 = addr;
    return v;
  }
Value8 InstructionDecoder::Load8(u16 addr) {
    Value8 v;
    v.type = Value8::Ld8;
    v.addr = addr;
    return v;
  }
Value8 InstructionDecoder::Load8(Register16 addr) {
    Value8 v;
    v.type = Value8::LdReg8;
    v.reg16 = addr;
    return v;
  }
Value8 InstructionDecoder::IO(Value8 port) {
    Value8 v = port;
    if (port.type == Value8::REG8) v.type = Value8::IO_R8;
    else if (port.type == Value8::IMM8) v.type = Value8::IO_I8;
    else { error = 1; }
    return v;
  }
  // Value8  Load(Register8  addr);
  
Value16 InstructionDecoder::AddSP(u8 offset) { return Value16::SP_offset(offset); }

void InstructionDecoder::decode() {
  pc_start = pc;
  u16 op = mmu->get(pc++);
  if (op == 0xCB) op = 0x100 | mmu->get(pc++);
  ii.PC_ptr = &pc;
  ii.PC_start_ptr = &pc_start;
  switch(op) {
#define LOG(m, op1, op2) do { if (pc_start > 0xFF) log(pc_start, #m, #op1, #op2); } while (false)
#define LOG(m, op1, op2)
#define ENTRY0(op, size, cycles, cycles2, flags, mnemonic, op1, op2) \
    case op: LOG(mnemonic, op1, op2); ii.mnemonic(); break;
#define ENTRY1(op, size, cycles, cycles2, flags, mnemonic, op1, op2) \
    case op: LOG(mnemonic, op1, op2); ii.mnemonic(op1); break;
#define ENTRY2(op, size, cycles, cycles2, flags, mnemonic, op1, op2) \
    case op: LOG(mnemonic, op1, op2); ii.mnemonic(op1, op2); break;
#include "opcodes.inc"

  default: log("unknown op", op);
  }
  ii.error++;
}
