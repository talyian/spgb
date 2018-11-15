// GBA Tests -- ported from Blargg's tests

#include "registers.hh"
#include "memory.hh"
#include "cpu.hh"

#include "registers.cc"
#include "instructions.cc"
#include "cpu.cc"

Registers registers;
Memory memory(0, 0);
OpPrinter printer;
CPU cpu(registers, memory);

void CHECKTEST(const char * testname, u16 pc) {
  printf("%-40s%s\n", testname,
         pc == 0xD000 ? "\x1b[1;31mFAIL\x1b[0m" :
         pc == 0xD008 ? "\x1b[1;32mOK\x1b[0m" :
         "\x1b[1;33mERR\x1b[0m");
}

int test_01_special_2_JR_negative() {
  const char src[] = (
    "\x3E\x00" // LD A, 0;
    "\xC3\x10\x01" // JP #jr_neg
    "\x3C" // INC A
    // JR_target
    "\x3C" // INC A
    "\x3C" // INC A
    "\xFE\x02" // CP 2
    "\xC2\x00\xD0" // JPNZ #TEST_FAIL
    "\xC3\x08\xD0" // JP   #TEST_SUCC
    "\x18\xF4"     // JR   JR_target
    );
  memcpy(memory.mem + 0x100, src, sizeof src);
  registers.PC = 0x100;
  OpParser<CPU> parser(registers, memory, cpu);
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    parser.Step();
  }
  CHECKTEST(__FUNCTION__, registers.PC);
  return 0;
}

int test_01_special_3_JR_positive() {
  const char src[] = (
    "\x3E\x00" // LD A, 0;
    "\x18\x01" // JP #jr_neg
    "\x3C" // INC A
    // JR_target
    "\x3C" // INC A
    "\x3C" // INC A
    "\xFE\x02" // CP 2
    "\xC2\x00\xD0" // JPNZ #TEST_FAIL
    "\xC3\x08\xD0" // JP   #TEST_SUCC
    );
  memcpy(memory.mem + 0x100, src, sizeof src);
  registers.PC = 0x100;
  OpParser<CPU> parser(registers, memory, cpu);
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    parser.Step();
  }
  CHECKTEST(__FUNCTION__, registers.PC);
  return 0;
}


int test_01_special_4_LD_PC_HL() {
  const char src[] = (
    "\x21\x11\x01" // LD HL, JUMP_HERE_ADDR
    "\x3E\x00"     // LD A, 0;
    "\xE9"         //  JP (HL)
    "\x3C"         // INC A
    // JUMP HERE
    "\x3C"         // INC A
    "\x3C"         // INC A
    "\xFE\x02" // CP 2
    "\xC2\x00\xD0" // JPNZ #TEST_FAIL
    "\xC3\x08\xD0" // JP   #TEST_SUCC
    "\x07\x01"     // JUMP_HERE_ADDR
    );
  memcpy(memory.mem + 0x100, src, sizeof src);
  registers.PC = 0x100;
  OpParser<CPU> parser(registers, memory, cpu);
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    parser.Step();
  }
  CHECKTEST(__FUNCTION__, registers.PC);
  return 0;
}

int test_01_special_5_POP_AF() {
  const char src[] = (
    "\x01\x00\x12" // LD BC $1200
    "\xC5"         // PUSH BC
    "\xF1"         // POP AF
    "\xF5"         // PUSH AF
    "\xD1"         // POP DE
    "\x79"         // LD A,C
    "\xE6\xF0"     // AND $F0
    "\xBB"         // CP E
    "\xC2\x00\xD0" // JPNZ #TEST_FAIL
    "\x04" // INC B
    "\x0C" // INC C
    "\x20\xF1" // JRNZ @PUSH BC
    "\xC3\x08\xD0" // JP   #TEST_SUCC
  );
  memcpy(memory.mem + 0x100, src, sizeof src);
  registers.PC = 0x100;
  OpParser<CPU> parser(registers, memory, cpu);
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    parser.Step();
  }
  CHECKTEST(__FUNCTION__, registers.PC);
  return 0;
}

struct {
  uint32_t sum;
  void reset() { sum = ~0; }
  void update(u8 new_byte) {
    sum ^= new_byte;
    for(int h = 8; h-->0;) {
      if (sum & 1) {
        sum >>= 1;
        sum ^= 0xEDB88320;
      }
      else sum >>= 1;
    }
  }
} crc;

int test_01_special_6_DAA() {
  const char src[] = (
    "\x11\x00\x00" // LD DE 0
    "\xD5" // push de
    "\xF1" // POP AF
    "\x27" // DAA
    "\xF5" // PUSH AF
    "\xCD\x00\x04" // TODO update crc
    "\xE1" // POP HL
    "\x7D" // LD A,L
    "\xCD\x00\x04" // TODO update crc
    "\x14" // INC D
    "\xC2\x03\x01" // JP NZ "push de"
    "\x7B" // LD A,E
    "\xC6\x10" // ADD 10
    "\x5F" // LD E,A
    "\xC2\x03\x01" // JP NZ "push de"
    "\xC3\x00\xD0" // JP   #TEST_FAIL
    );
  memcpy(memory.mem + 0x100, src, sizeof src);
  memory.mem[0x400] = 0xC9; // ret
  registers.PC = 0x100;
  OpParser<CPU> parser(registers, memory, cpu);

  crc.reset();
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    if (registers.PC == 0x400) { crc.update(registers.A); }
    parser.Step();
  }
  CHECKTEST(__FUNCTION__, ~crc.sum == 0x6A9F8D8A ? 0xD008 : 0xD000);
  return 0;
}

int test_06_ld_r_r () {
  // calls test with 00 10 e0 f0
  const char test_instr[] = (
    "\x0E\x00" // LD C 00
    "\xCD\x00\x00" // TODO call test
    "\x0E\x10" // LD C 00
    "\xCD\x00\x00" // TODO call test
    "\x0E\xE0" // LD C 00
    "\xCD\x00\x00" // TODO call test
    "\x0E\xF0" // LD C 00
    "\xCD\x00\x00" // TODO call test
  );
  const char test[] = (

  );
  crc.reset();
  if (checksum_af_bc_de_hl) {
    crc.update(registers.A);
    crc.update(registers.F);
    crc.update(registers.B);
    crc.update(registers.C);
    crc.update(registers.D);
    crc.update(registers.E);
    crc.update(registers.H);
    crc.update(registers.L);
  }
  printf("checksum: %x\n", ~crc.sum);
  return 0;
}

int main() {
  test_01_special_2_JR_negative();
  test_01_special_3_JR_positive();
  test_01_special_4_LD_PC_HL();
  test_01_special_5_POP_AF();
  test_01_special_6_DAA();
  return 0;
}

void abort(uint32_t n) { exit(n); }
