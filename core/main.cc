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
  rom_path = "data/opus5.gb";
  // rom_path = "data/ttt.gb";
  // rom_path = "data/bgbtest.gb";
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

  // OpPrinter printer;
  OpParser<CPU> pp(registers, memory, exec);
  // OpParser<OpPrinter> pprinter(registers, memory, printer);
  Debugger debugger(registers, memory, ppu);
  Keypad keys { registers, memory };

  // Disassemble(pprinter);
  // return 0;
  uint64_t ticks = 0;
  for(u8 ct = 0; ; !(ct++) ? (usleep(100), 0) : 0) {
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

    if (!exec.halted) {
      debugger.Step();
      pp.Step();
      checkDMA(registers, memory);
      ticks += exec.timer;
    }
    keys.Step();
    ppu.Step(4 * exec.timer); // TODO: why do I have to multiply by 4? is this the T-clock multiplier?
  }
}
