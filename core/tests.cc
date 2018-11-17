// GBA Tests -- ported from Blargg's tests

#include "registers.hh"
#include "memory.hh"
#include "cpu.hh"

#include "registers.cc"
#include "instructions.cc"
#include "cpu.cc"
#include "timer.cc"

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
    "\x21\x07\x01" // LD HL, JUMP_HERE_ADDR
    "\x3E\x00"     // LD A, 0;
    "\xE9"         //  JP (HL)
    "\x3C"         // INC A
    // JUMP HERE
    "\x3C"         // INC A
    "\x3C"         // INC A
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

void test_02_interrupts_02_EI() {
  // tests that when you LD(FF0F) it just jumps to the interrupt right away
  // and that the stack is set appropriately
  const char src[] = (
    "\xFB"         // EI
    "\x01\x00\x00" // LD BC $0
    "\xC5"         // PUSH BC
    "\xC1"         // POP BC
    "\x04"         // INC B
    // enable timer interrupt
    "\x3E\x04"     // LD A,$4
    "\xE0\x0F"     // LD (FF0F),A
    // interrupt_addr
    "\x05"         // DEC B
    "\xC2\x00\xD0" // JPNZ test_failed
    "\xF8\xFE"     // LD HL,SP-2
    "\x2A"         // LDI A,(HL)
    "\xFE\x0B"     // CP A, $0B
    "\xC2\x00\xD0" // JPNZ test_failed
    "\x7E"         // LD A,(HL)
    "\xFE\x01"     // CP A, $01
    "\xC2\x00\xD0" // JPNZ test_failed
    "\xF0\x0F"     // ld a, FFOF
    "\xE6\x04"     // AND $4
    "\xC2\x00\xD0" // JPNZ test_failed
    "\xC3\x08\xD0" // JP   #TEST_SUCC
  );
  memory.exit_bios = true;
  memcpy(memory.mem + 0x100, src, sizeof src);
  memcpy(memory.mem + 0x50, "\x3C\xc9", 2); // inc a; ret in timer interrupt
  registers.PC = 0x100;
  OpParser<CPU> parser(registers, memory, cpu);
  memory[0xFFFF] = 0xFF;
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    u8 active_interrupts = registers.IME & memory[0xFFFF] & memory[0xFF0F];
    if (active_interrupts)
    {
      cpu.halted = false;
      registers.IME = 0;
      memory[0xFF0F] = 0;
      cpu.timer += 12;
      if (active_interrupts & 0x1) { cpu.RST(0x40); } // VBLANK
      else if (active_interrupts & 0x2) { cpu.RST(0x48); } // LCDC
      else if (active_interrupts & 0x4) { cpu.RST(0x50); } // TIMER
      else if (active_interrupts & 0x8) { cpu.RST(0x58); } // SERIAL
      else if (active_interrupts & 0x10) { cpu.RST(0x60); } // KEYPAD
      else ;
    }
    parser.Step();
  }
  CHECKTEST(__FUNCTION__, registers.PC);
}

void test_02_interrupts_03_DI() {
  // null test for previous test
  const char src[] = (
    "\xF3"         // DI
    "\x01\x00\x00" // LD BC $0
    "\xC5"         // PUSH BC
    "\xC1"         // POP BC
    // enable timer interrupt
    "\x3E\x04"     // LD A,$4
    "\xE0\x0F"     // LD (FF0F),A
    // interrupt_addr
    "\xF8\xFE"     // LD HL,SP-2
    "\x2A"         // LDI A,(HL)
    "\xB6"         // OR (HL)
    "\xC2\x00\xD0" // JPNZ test_failed
    "\xF0\x0F"     // ld a, FFOF
    "\xE6\x04"     // AND $4
    "\xCA\x00\xD0" // JPZ test_failed
    "\xC3\x08\xD0" // JP   #TEST_SUCC
  );
  memory.exit_bios = true;
  memcpy(memory.mem + 0x100, src, sizeof src);
  memcpy(memory.mem + 0x50, "\x3C\xc9", 2); // inc a; ret in timer interrupt
  registers.PC = 0x100;
  OpParser<CPU> parser(registers, memory, cpu);
  memory[0xFFFF] = 0xFF;
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    u8 active_interrupts = registers.IME & memory[0xFFFF] & memory[0xFF0F];
    if (active_interrupts)
    {
      cpu.halted = false;
      registers.IME = 0;
      memory[0xFF0F] = 0;
      cpu.timer += 12;
      if (active_interrupts & 0x1) { cpu.RST(0x40); } // VBLANK
      else if (active_interrupts & 0x2) { cpu.RST(0x48); } // LCDC
      else if (active_interrupts & 0x4) { cpu.RST(0x50); } // TIMER
      else if (active_interrupts & 0x8) { cpu.RST(0x58); } // SERIAL
      else if (active_interrupts & 0x10) { cpu.RST(0x60); } // KEYPAD
      else ;
    }
    parser.Step();
  }
  CHECKTEST(__FUNCTION__, registers.PC);
}

void test_02_interrupts_04_timer_doesnt_work() {
  const char src[] = (
    "\x3E\x05"     // LD A,$5;
    "\xE0\x07"     // FF07=$5; // enable timer at every 16 cycles
    "\x3E\x00"
    "\xE0\x05"     // FF05 = 0
    "\xE0\x0F"     // FF0F = 0

    // Delay 500
    "\xF5"         // PUSH AF
    "\x3E\x01"
    "\xCD\x00\x02" // Call 200
    // line 110
    "\x3E\xCC"
    "\xCD\x00\x03" // Call 300
    "\xF1"         // POP AF

    "\xF0\x0F"     // LD A, (FF0F)
    // line 118
    "\xF5\x3E\x01\xCD\x00\x02\x3E\xCC\xCD\x00\x03\xF1" // delay 500

    "\xE6\x04"     // AND $4
    "\xC2\x00\xD0" // JPNZ test_failed
    "\xF5\x3E\x01\xCD\x00\x02\x3E\xCC\xCD\x00\x03\xF1" // delay 500

    "\xF0\x0F"     // LD A, (FF0F)
    "\xE6\x04"     // AND $4
    "\xCA\x00\xD0" // JPZ test_failed
    "\xC3\x08\xD0" // JP   #TEST_SUCC
  );
  const char delay_func_1[] = (
    "\xF5\x3E\xDF"
    "\xCD\x00\x03"
    "\xF1\x3D"
    "\x20\xF6\xC9"
  );
  const char delay_func_2[] = (
    "\xD6\x05"
    "\x30\xFC"
    "\x1F"
    "\x30\x00"
    "\xCE\x01"
    "\xD0"
    "\xC8"
    "\x00\xC9"
  );

  memory.exit_bios = true;
  memcpy(memory.mem + 0x100, src, sizeof src);
  memcpy(memory.mem + 0x200, delay_func_1, sizeof delay_func_1);
  memcpy(memory.mem + 0x300, delay_func_2, sizeof delay_func_2);
  memcpy(memory.mem + 0x50, "\x3C\xc9", 2); // inc a; ret in timer interrupt
  registers.PC = 0x100;
  Timer timer { memory };
  OpParser<CPU> parser(registers, memory, cpu);
  memory[0xFFFF] = 0xFF;
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    u8 active_interrupts = registers.IME & memory[0xFFFF] & memory[0xFF0F];
    if (active_interrupts)
    {
      cpu.halted = false;
      registers.IME = 0;
      memory[0xFF0F] = 0;
      cpu.timer += 12;
      if (active_interrupts & 0x1) { cpu.RST(0x40); } // VBLANK
      else if (active_interrupts & 0x2) { cpu.RST(0x48); } // LCDC
      else if (active_interrupts & 0x4) { cpu.RST(0x50); } // TIMER
      else if (active_interrupts & 0x8) { cpu.RST(0x58); } // SERIAL
      else if (active_interrupts & 0x10) { cpu.RST(0x60); } // KEYPAD
      else ;
    }
    parser.Step();
    timer.Step(2 * cpu.timer); // but why
  }
  CHECKTEST(__FUNCTION__, registers.PC);
}

void test_02_interrupts_05_halt() {
  const char src[] = (
    "\x3E\x05\xE0\x07" // (ff07) = 5 - timer enable, 1024 cycles per timer tick
    "\x3E\x00\xE0\x05" // (ff05) = 0 - start timer at 0
    "\x3E\x00\xE0\x0F" // (ff0F) = 0 - clear interrupts
    "\x76\x00" // HALT
    "\xC3\x08\xD0" // JP   #TEST_SUCC
  );
  memory.exit_bios = true;
  memcpy(memory.mem + 0x100, src, sizeof src);
  memcpy(memory.mem + 0x50, "\x3C\xc9", 2); // inc a; ret in timer interrupt
  registers.PC = 0x100;
  Timer timer { memory };
  OpParser<CPU> parser(registers, memory, cpu);
  memory[0xFFFF] = 0xFF;
  registers.IME = 0xFF;
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    u8 active_interrupts = registers.IME & memory[0xFFFF] & memory[0xFF0F];
    if (active_interrupts)
    {
      cpu.halted = false;
      registers.IME = 0;
      memory[0xFF0F] = 0;
      cpu.timer += 12;
      if (active_interrupts & 0x1) { cpu.RST(0x40); } // VBLANK
      else if (active_interrupts & 0x2) { cpu.RST(0x48); } // LCDC
      else if (active_interrupts & 0x4) { cpu.RST(0x50); } // TIMER
      else if (active_interrupts & 0x8) { cpu.RST(0x58); } // SERIAL
      else if (active_interrupts & 0x10) { cpu.RST(0x60); } // KEYPAD
      else ;
    } else if (memory[0xFF0F] & memory[0xFFFF]) {
      // halt while DI can get resumed, the handler just won't run
      cpu.halted = false;
    }
    if (!cpu.halted) {
      parser.Step();
    }
    timer.Step(2 * cpu.timer);
  }
  CHECKTEST(__FUNCTION__, registers.PC);
}

void test_03_op_sp_hl_100_SP_check() {
  const char src[] = (
    "\x31\x00\x0F" // LD SP $f00
    "\xE8\x01"     // ADD SP, 1
    "\xE8\xFF"     // ADD SP, -1
    "\xE8\xFE"     // ADD SP, -2
    "\xC3\x08\xD0" // JP   #TEST_SUCC
    );
  memcpy(memory.mem + 0x100, src, sizeof src);
  registers.PC = 0x100;
  OpParser<CPU> parser(registers, memory, cpu);
  while(registers.PC != 0xD000 && registers.PC != 0xD008) {
    parser.Step();
    registers.dump();
  }
  CHECKTEST(__FUNCTION__, registers.PC);
}


int main() {
  test_01_special_2_JR_negative();
  test_01_special_3_JR_positive();
  test_01_special_4_LD_PC_HL();
  test_01_special_5_POP_AF();
  test_01_special_6_DAA();

  test_02_interrupts_02_EI();
  test_02_interrupts_03_DI();
  test_02_interrupts_04_timer_doesnt_work();
  test_02_interrupts_05_halt();

  test_03_op_sp_hl_100_SP_check();
  return 0;
}

void abort(uint32_t n) { exit(n); }
