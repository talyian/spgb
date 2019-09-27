#include "wasm_host.hpp"
#include "instructions.hpp"
#include "instruction_decoder.hpp"
#include "instruction_printer.hpp"
#include "instruction_runner.hpp"
#include "memory_mapper.hpp"
#include "ppu.hpp"

#include "boot_rom.hpp"
#include "data_bgbtest_gb.hpp"

#ifndef SEPARATE_COMPILATION
#include "platform_utils.cc"
#include "instruction_decoder.cpp"
#include "instructions.cpp"
#endif


struct emulator_t {
  u8 rom[0x8000];
  u8 ram[0x8000];
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
}
