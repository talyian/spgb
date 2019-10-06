#include "base.hpp"
#include "instruction_decoder.hpp"

u8 InstructionDecoderBase::Imm8() { return mmu->get(pc++); }
i8 InstructionDecoderBase::ImmI8() { return mmu->get(pc++); }
u16 InstructionDecoderBase::Imm16() {
  u16 v = mmu->get(pc++);
  v = v + 256 * mmu->get(pc++);
  return v;
}

Value16 InstructionDecoderBase::Load16(Value16 addr) { error = 100; return {0}; }
Value8 InstructionDecoderBase::Inc8(Register16 addr) { return Value8::_Inc(addr); }
Value8 InstructionDecoderBase::Dec8(Register16 addr) { return Value8::_Dec(addr); }
Value8 InstructionDecoderBase::Load8(u16 addr) { return Value8::_Load(addr); }
Value8 InstructionDecoderBase::Load8(Register16 addr) { return Value8::_Load(addr); }
Value8 InstructionDecoderBase::IO(u8 port) { return Value8::_Io(port); }
Value8 InstructionDecoderBase::IO(Register8 port) { return Value8::_Io(port); }
Value16 InstructionDecoderBase::AddSP(i8 offset) { return Value16::SP_offset(offset); }

