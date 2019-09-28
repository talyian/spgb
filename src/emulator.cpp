#include "emulator.hpp"
#include "boot_rom.hpp"
#include "data_bgbtest_gb.hpp"

emulator_t::emulator_t() : mmu(rom, ram) {
  mmu.bios_rom = DMG_ROM_bin;
  u8 * _gb = ___data_bgbtest_gb;
  int _gb_len = ___data_bgbtest_gb_len;
  mmu.load_cart(_gb, _gb_len);
  
  decoder.mmu = &mmu;
  decoder.ii.mmu = &mmu;
  ppu.memory = &mmu;
  printer.mmu = &mmu;
  // printer.ii.mmu = &mmu;
}

u32 emulator_t::single_step() {
  u8 interrupt = (mmu.get(0xFFFF) & mmu.get(0xFF0F));
  if (decoder.ii.ime && interrupt) {
    if (interrupt == 1) {
      decoder.ii.ime = 0;
      decoder.ii.halted = false;
      mmu.get_ref(0xFF0F) &= ~0x1;
      log("pushing", decoder.pc, "for VBLANK");
      decoder.ii._push(decoder.pc);
      decoder.pc = 0x40;
    } else {
      log("interrupt", interrupt);
    }
  }
  if (decoder.error) { log(decoder.pc_start, "decoder error"); _stop(); }
  if (decoder.ii.error) { log(decoder.pc_start, "runner error"); _stop(); }

  if (!decoder.ii.halted) {
    decoder.decode();
  }

  for(int i=0; i<break_n; i++) {
    if (breakpoints[i] == decoder.pc) {
      is_debugging = true;
      return 0;
    }
  }
  
  u32 dt = 16;              // TODO: do timing based on actual instruction decodetime
  ppu.tick(dt);
  return dt;
}
void emulator_t::step(i32 ticks) {
  while(ticks > 0) {
    if (is_debugging) {
      printer.pc = decoder.pc;
      _log(decoder.pc);
      printer.decode();
      
      decoder.ii.dump();
      break;
    }
    ticks -= single_step();
    if (is_stepping) { is_stepping = false; is_debugging = true; } 
  }
}
