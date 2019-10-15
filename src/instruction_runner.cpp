#include "instruction_runner.hpp"

void InstructionRunner::dump() {
  log(". . . . . . . . . A  F  B  C  D  E  HL   SP   PC");
  log(". . . . . . . . .", cpu.registers.A, cpu.registers.F, cpu.registers.B,
      cpu.registers.C, cpu.registers.D, cpu.registers.E, cpu.registers.HL,
      cpu.registers.SP, *PC_start_ptr);
}

void InstructionRunner::_push(Reg16 value) {
  mmu->set(--cpu.registers.SP, value.h);
  mmu->set(--cpu.registers.SP, value.l);
}

u16 InstructionRunner::_pop() {
  u16 l = mmu->get(cpu.registers.SP++);
  u16 h = mmu->get(cpu.registers.SP++);
  return h * 0x100 + l;
}

u8 InstructionRunner::_read8_addr(u16 addr) { return mmu->get(addr); }

u8 InstructionRunner::_read8(Value8 v) {
  switch (v.type) {
  case Value8::IMM8:
    return v.value;
  case Value8::REG8:
    return _read8(v.reg);
  case Value8::Ld8Reg:
    return _read8_addr(_read16(v.reg16));
  case Value8::Ld8Imm:
    return _read8_addr(v.addr);
  case Value8::IoImm8:
    return _read8_addr(0xFF00 + v.value);
  case Value8::IoReg8:
    return _read8_addr(0xFF00 + _read8(v.reg));
  case Value8::Ld8Dec: {
    auto addr = _read16(v.reg16);
    auto value = _read8_addr(addr);
    _write16(v.reg16, addr - 1);
    return value;
  }
  case Value8::Ld8Inc: {
    auto addr = _read16(v.reg16);
    auto value = _read8_addr(addr);
    _write16(v.reg16, addr + 1);
    return value;
  }
  }
}

u8 InstructionRunner::_read8(Register8 r) {
  switch (r) {
#define X(RR)                                                                  \
  case Register8::RR:                                                          \
    return cpu.registers.RR;                                                   \
    break;
    X(A);
    X(B);
    X(C);
    X(D);
    X(E);
    X(F);
    X(H);
    X(L);
#undef X
  }
}

void InstructionRunner::_write8(Value8 target, u8 value) {
  switch (target.type) {
  case Value8::IMM8: {
    log("error-write-to-imm");
    error = 100;
    return;
  }
  case Value8::REG8: {
    _write8(target.reg, value);
    return;
  }
  case Value8::IoImm8: {
    _write8_addr(0xFF00 + target.value, value);
    return;
  }
  case Value8::IoReg8: {
    _write8_addr(0xFF00 + _read8(target.reg), value);
    return;
  }
  case Value8::Ld8Reg: {
    _write8_addr(_read16(target.reg16), value);
    return;
  }
  case Value8::Ld8Inc: {
    u16 addr = _read16(target.reg16);
    _write8_addr(addr, value);
    _write16(target.reg16, addr + 1);
    return;
  }
  case Value8::Ld8Dec: {
    u16 addr = _read16(target.reg16);
    _write8_addr(addr, value);
    _write16(target.reg16, addr - 1);
    return;
  }
  case Value8::Ld8Imm: {
    _write8_addr(target.addr, value);
    return;
  }
  }
}
void InstructionRunner::_write8(Register8 target, u8 value) {
  switch (target) {
#define X(RR)                                                                  \
  case Register8::RR:                                                          \
    cpu.registers.RR = value;                                                  \
    break;
    X(A);
    X(B);
    X(C);
    X(D);
    X(E);
    X(F);
    X(H);
    X(L);
#undef X
  }
}
void InstructionRunner::_write8_addr(u16 addr, u8 value) {
  // sprite table 1
  // if (0x8000 <= addr && addr < 0x8800) {
  //   log(*PC_start_ptr, addr, "<-", value);
  // }

  // if (*PC_start_ptr > 0x100) {
  // // if (addr == 0xFF42) { log(*PC_start_ptr, "scroll y", value); }
  // // if (addr == 0xFF43) { log(*PC_start_ptr, "scroll x", value); }
  // }
  // if (0xFF00 <= addr && addr < 0xFF80) _handle_io_write(addr, value);
  mmu->set(addr, value);
}

u16 InstructionRunner::_read16_addr(u16 addr) {
  u16 value = mmu->get(addr++);
  return value + mmu->get(addr) * 0x100;
}
u16 InstructionRunner::_read16(Register16 r) {
  switch (r) {
  case Register16::BC:
    return cpu.registers.BC;
    break;
  case Register16::DE:
    return cpu.registers.DE;
    break;
  case Register16::HL:
    return cpu.registers.HL;
    break;
  case Register16::SP:
    return cpu.registers.SP;
    break;
  case Register16::AF:
    return cpu.registers.AF;
    break;
  }
}
u16 InstructionRunner::_read16(Value16 v) {
  switch (v.type) {
  case Value16::IMM16:
    return v.value;
  case Value16::REG16:
    return _read16(v.reg);
  case Value16::SP_d8: {
    u16 sp = cpu.registers.SP;
    u16 offset = (i8)v.offset;
    u16 sp_2 = sp + offset;
    cpu.flags.Z = 0;
    cpu.flags.N = 0;
    cpu.flags.H = (sp & 0xF) + (offset & 0xF) > 0xF;
    cpu.flags.C = ((sp & 0xFF) + (v.offset & 0xFF)) > 0xFF;
    return sp + (i8)v.offset;
  }
  }
}

void InstructionRunner::_write16(Register16 r, u16 value) {
  switch (r) {
  case Register16::BC:
    cpu.registers.BC = value;
    break;
  case Register16::DE:
    cpu.registers.DE = value;
    break;
  case Register16::HL:
    cpu.registers.HL = value;
    break;
  case Register16::SP:
    cpu.registers.SP = value;
    break;
  case Register16::AF:
    cpu.registers.AF = value & 0xFFF0;
    break;
  }
}

void InstructionRunner::_write16_addr(u16 addr, u16 value) {
  mmu->set(addr++, value);
  mmu->set(addr, value >> 8);
}

void InstructionRunner::_write16(Value16 target, u16 value) {
  switch (target.type) {
  case Value16::IMM16:
    return _write16_addr(target.value, value);
  case Value16::REG16:
    return _write16(target.reg, value);
  case Value16::SP_d8:
    return _write16_addr((u16)((u16)cpu.registers.SP + (i8)target.offset),
                         value);
  }
}
