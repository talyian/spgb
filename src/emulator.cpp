#include "emulator.hpp"
#include "data/dmg_boot.hpp"
#include "data/bgbtest.hpp"
// #include "data/04-op r,imm.gb.hpp"
// #include "data/05-op rp.gb.hpp"
// #include "data/06-ld r,r.gb.hpp"

emulator_t::emulator_t(u8 * cart_data, u32 cart_len): mmu(rom, ram) {
  mmu.bios_rom = DMG_ROM_bin;
  decoder.mmu = &mmu;
  decoder.ii.mmu = &mmu;
  ppu.memory = &mmu;
  printer.mmu = &mmu;
  load_cart(cart_data, cart_len);
}

emulator_t::emulator_t() : emulator_t(___data_bgbtest_gb, ___data_bgbtest_gb_len) { }

void emulator_t::load_cart(u8 * cart_data, u32 cart_len) {
  this->cart_data = cart_data;
  this->cart_len = cart_len;
  this->cpu.clear();
  this->mmu.bios_active = true;
  this->mmu.clear();
  this->decoder.pc = this->decoder.pc_start = 0;
  mmu.load_cart(cart_data, cart_len);
}

u32 emulator_t::single_step() {
  u8 interrupt = (mmu.get(0xFFFF) & mmu.get(0xFF0F));
  if (cpu.IME && interrupt) {
    // when an interrupt triggers we clear the IME
    // and the flag that triggered
    // and restart from a halting state
    cpu.IME = 0;
    cpu.halted = false;
    if (interrupt == 1) {
      mmu.get_ref(0xFF0F) &= ~0x1;
      decoder.ii._push(decoder.pc);
      decoder.pc = 0x40;
    } else {
      log("interrupt", interrupt);
    }
    // TODO: does this actually take zero time?    
    return 0;
  }
  if (decoder.error) { log(decoder.pc_start, "decoder error"); _stop(); }
  if (decoder.ii.error) { log(decoder.pc_start, "runner error"); _stop(); }

  joypad.tick();
  
  if (mmu.get(IO::DMA)) {
    // technically this won't work if we transfer from 0
    // but that seems highly unlikely
    dma_transfer(&mmu, mmu.get(IO::DMA));
    mmu.set(IO::DMA, 0);
  }

  if (!cpu.halted) {
    decoder.decode();
  }

  u32 dt = 16;              // TODO: do timing based on actual instruction decodetime
  dt = 8;
  ppu.tick(dt);
  return dt;
}
void emulator_t::step(i32 ticks) {
  // _runner.verbose_log = true;
  while(ticks > 0) {
    debug.step();

    // is_debugging means we don't run any code
    if (debug.is_debugging) {
      printer.pc = decoder.pc_start;
      decoder.ii.dump();
      _log("breakpoint");
      _log(decoder.pc_start);
      printer.decode();
      _stop(); // stop core run loop
      break;
    }
    ticks -= single_step();

    // stepping means we run one instruction and go back into debug
    if (debug.is_stepping) {
      debug.is_stepping = false;
      debug.is_debugging = true;
    } 
  }
}

