#include <cstdint>
#include <cstdio>
#include <csignal>
#include <cstdlib>

#include "utils.hh"
#include "utils.cc"

#include "registers.hh"
#include "registers.cc"

#include "instructions.hh"
#include "instructions.cc"

#include "cpu.hh"
#include "cpu.cc"

#include "disassemble.cc"
#include "ui_remote.hh"
#include "ppu.cc"
#include "dma.cc"
#include "debug.cc"

#include "main.hh"

int main(int argc, const char ** argv) {
  const char * rom_path = "gb-test-roms/cpu_instrs/individual/06-ld r,r.gb";
  rom_path = "tools/gb-test-roms/cpu_instrs/individual/01-special.gb";
  // rom_path = "data/opus5.gb";
  // rom_path = "data/bgbtest.gb";
  // rom_path = "data/ttt.gb";
  if (argc == 2) rom_path = argv[1];

  Registers registers;
  Memory memory (
    fopen("data/DMG_ROM.bin", "rb"),
    fopen(rom_path, "rb")
    // fopen("data/opus5.gb", "r")
    // fopen("data/bgbtest.gb", "rb")
  );

  CPU exec { registers, memory };
  PPU ppu { memory };

  memory.exit_bios = 0x1;
  registers.AF = 0x01B0;
  registers.BC = 0x0013;
  registers.DE = 0x00D8;
  registers.HL = 0x014D;
  registers.PC = 0x0100;
  registers.SP = 0xFFFE;
  ppu.clock = 0x28;
  ppu.LY = 0;

  // OpPrinter printer;
  OpParser<CPU> pp(registers, memory, exec);
  // OpParser<OpPrinter> pprinter(registers, memory, printer);
  Debugger debugger(registers, memory, ppu);
  Keypad keys { registers, memory };

  // is_stepping = true;
  // Disassemble(pprinter);
  // return 0;
  uint64_t ticks = 0;
  for(u8 ct = 0; ; !(ct++) ? (usleep(10), 0) : 0) {
  // for(;;) {
    u8 active_interrupts = registers.IME & memory[0xFFFF] & memory[0xFF0F];
    if (active_interrupts)
    {
      exec.halted = false;
      registers.IME = 0;
      memory[0xFF0F] = 0;
      exec.timer += 12;
      if (active_interrupts & 0x1) { exec.RST(0x40); } // VBLANK
      else if (active_interrupts & 0x2) { exec.RST(0x48); } // LCDC
      else if (active_interrupts & 0x4) { exec.RST(0x50); } // TIMER
      else if (active_interrupts & 0x8) { exec.RST(0x58); } // SERIAL
      else if (active_interrupts & 0x10) { exec.RST(0x60); } // KEYPAD
      else ;
    }
    debugger.Step();
    if (!exec.halted) {
      pp.Step();
      checkDMA(registers, memory);
      ticks += exec.timer;
    }
    keys.Step();
    ppu.Step(exec.timer); // TODO: why do I have to multiply by 4? is this the T-clock multiplier?

    // serial
    // if (memory[0xFF02 & 0x80]) {
    //   if (memory[0xFF01]) printf("Serial: %c\n", memory[0xFF01]);
    //   // TODO: this needs to be delayed by 8 instruction cycles
    //   memory[0xFF02] &= ~0x80;
    // }
  }
}
