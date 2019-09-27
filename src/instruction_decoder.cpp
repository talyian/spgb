#include "base.hpp"
#include "instruction_decoder.hpp"

u8 InstructionDecoder::Imm8() { return mmu->get(pc++); }
u8 InstructionDecoder::ImmI8() { return mmu->get(pc++); }
u16 InstructionDecoder::Imm16() {
  u16 v = mmu->get(pc++);
  v = v + 256 * mmu->get(pc++);
  return v;
}

Value16 InstructionDecoder::Load16(Value16 addr) { error = 100; return {0}; }
Value8 InstructionDecoder::Inc8(Register16 addr) { return Value8::_Inc(addr); }
Value8 InstructionDecoder::Dec8(Register16 addr) { return Value8::_Dec(addr); }
Value8 InstructionDecoder::Load8(u16 addr) { return Value8::_Load(addr); }
Value8 InstructionDecoder::Load8(Register16 addr) { return Value8::_Load(addr); }
Value8 InstructionDecoder::IO(u8 port) { return Value8::_Io(port); }
Value8 InstructionDecoder::IO(Register8 port) { return Value8::_Io(port); }
  
Value16 InstructionDecoder::AddSP(u8 offset) { return Value16::SP_offset(offset); }

void InstructionDecoder::decode() {
  pc_start = pc;
  u16 op = mmu->get(pc++);
  if (op == 0xCB) op = 0x100 | mmu->get(pc++);
  ii.PC_ptr = &pc;
  ii.PC_start_ptr = &pc_start;
  switch(op) {
#define ENTRY0(op, size, cycles, cycles2, flags, mnemonic, op1, op2) \
    case op: ii.mnemonic(); break;
#define ENTRY1(op, size, cycles, cycles2, flags, mnemonic, op1, op2) \
    case op: ii.mnemonic(op1); break;
#define ENTRY2(op, size, cycles, cycles2, flags, mnemonic, op1, op2) \
    case op: ii.mnemonic(op1, op2); break;
#include "opcodes.inc"

  default: log("unknown op", op);
  }
  ii.error++;
}
