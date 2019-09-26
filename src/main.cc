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
  void step() {
    if (decoder.error) { log("decoder error"); _stop(); }
    if (decoder.ii.error) { log("runner error"); _stop(); }
    decoder.decode();
    ppu.tick(32); // TODO: timing shoudl we be using
  }
};
#ifdef WASM
extern "C" void _check_memory_size();
extern "C" void WASM_EXPORT test_memory() {
  _check_memory_size();
  log("__builtin_wasm_memory_size", (u32)__builtin_wasm_memory_size(0));
  __builtin_wasm_memory_grow(0, 3);
  log("growing 3");
  _check_memory_size();
  log("__builtin_wasm_memory_size", (u32)__builtin_wasm_memory_size(0));
}
#endif
extern "C" void * get_emulator() {
  return new emulator_t {};
}
extern "C" void step_frame(void * emulator_vptr) {
  auto emulator = (emulator_t*)emulator_vptr;
  for(int i = 0; i < 16 * 1024; i++) {
    emulator->step();
  }
  // _push_frame(0x100, mmu.vram, 0x800);
  // _push_frame(0x101, mmu.vram + 0x800, 0x800);
  // _push_frame(0x102, mmu.vram + 0x1000, 0x800);
  // _push_frame(0x200, mmu.vram + 0x1800, 0x400);
  // _push_frame(0x201, mmu.vram + 0x1C00, 0x400);
}
