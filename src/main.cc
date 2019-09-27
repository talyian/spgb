#include "wasm_host.hpp"
#include "boot_rom.hpp"
#include "instructions.hpp"
#include "instruction_decoder.hpp"
#include "instruction_printer.hpp"
#include "instruction_runner.hpp"
#include "memory_mapper.hpp"
#include "ppu.hpp"

#include "data_bgbtest_gb.hpp"

u8 rom[0x8000];
u8 ram[0x8000];

struct emulator_t {
  MemoryMapper mmu;
  InstructionDecoder decoder {0};
  PPU ppu {};
  
  emulator_t() : mmu(rom, ram) {
    mmu.bios_rom = DMG_ROM_bin;
    u8 * _gb = ___data_bgbtest_gb;
    int _gb_len = ___data_bgbtest_gb_len;
    mmu.load_cart(_gb, _gb_len);

    decoder.mmu = &mmu;
    decoder.ii.mmu = &mmu;
    ppu.memory = &mmu;
  }
  void step(i32 ticks) {
    while(ticks > 0) { 
      if (decoder.error) { log("decoder error"); _stop(); }
      if (decoder.ii.error) { log("runner error"); _stop(); }
      decoder.decode();
      u32 dt = 16; // TODO: do timing based on actual instruction decodetime
      ticks -= dt; 
      ppu.tick(dt);
    }
  }
};

extern "C" emulator_t * WASM_EXPORT get_emulator() {
  return new emulator_t {};
}
extern "C" void WASM_EXPORT step_frame(emulator_t * emulator) {
  #define CLOCK_HZ 8200000
  #define FPS 60
  emulator->step(CLOCK_HZ / FPS);
  // _push_frame(0x100, mmu.vram, 0x800);
  // _push_frame(0x101, mmu.vram + 0x800, 0x800);
  // _push_frame(0x102, mmu.vram + 0x1000, 0x800);
  // _push_frame(0x200, mmu.vram + 0x1800, 0x400);
  // _push_frame(0x201, mmu.vram + 0x1C00, 0x400);
}
