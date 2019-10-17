#pragma once
#include "base.hpp"
#include "emulator/mmu.hpp"
#include "instruction_printer.hpp"
#include "instruction_runner.hpp"

struct InstructionDecoderBase {
  u16 pc, pc_start;
  u16 error = 0;
  MemoryMapper *mmu=0;  

  u8 Imm8();
  i8 ImmI8();
  u16 Imm16();
  Value16 Load16(Value16 addr);
  Value8 Inc8(Register16 addr);
  Value8 Dec8(Register16 addr);
  Value8 Load8(u16 addr);
  Value8 Load8(Register16 addr);
  Value8 IO(Register8 port);
  Value8 IO(u8 port);
  Value16 AddSP(i8 offset);

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
  Conditions CC() { return Conditions::C; }
  Conditions CZ() { return Conditions::Z; }
  Conditions CT() { return Conditions::T; }
  Conditions CNC() { return Conditions::NC; }
  Conditions CNZ() { return Conditions::NZ; }  
};

template<class Handler>
struct InstructionDecoderT : InstructionDecoderBase {
  Handler &ii;
  InstructionDecoderT(Handler &hh) : ii(hh) { pc = 0; }
  
  void decode() {
    pc_start = pc;
    u16 op = mmu->get(pc++);
    if (op == 0xCB) op = 0x100 | mmu->get(pc++);
    ii.PC_ptr = &pc;
    ii.PC_start_ptr = &pc_start;

    u8 op_size = 0;
    u8 op_cycles = 0;
    switch(op) {
  #define ENTRY0(OP, size, cycles, cycles2, flags, MNEMONIC, op1, op2) \
      case OP: op_size = size; ii.MNEMONIC(); break;
  #define ENTRY1(OP, size, cycles, cycles2, flags, MNEMONIC, op1, op2) \
      case OP: op_size = size; ii.MNEMONIC(op1); break;
  #define ENTRY2(OP, size, cycles, cycles2, flags, MNEMONIC, op1, op2) \
      case OP: op_size = size; ii.MNEMONIC(op1, op2); break;
  #include "opcodes.inc"
    default: log("unknown op", op);
    }
  }
};
