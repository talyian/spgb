#include "emulator.hpp"
#include "data/dmg_boot.hpp"

emulator_t::emulator_t(u8 *cart_data, u32 cart_len) {
  mmu.bios_rom = DMG_ROM_bin;
  decoder.mmu = &mmu;
  decoder.ii.mmu = &mmu;
  ppu.mmu = &mmu;
  printer.mmu = &mmu;
  load_cart(cart_data, cart_len);
}

emulator_t::emulator_t() : emulator_t(0, 0) {}

void emulator_t::load_cart(u8 *cart_data, u32 cart_len) {
  this->cart = Cart{cart_data, cart_len};
  this->cpu.clear();
  this->mmu.BiosLock = 0;
  this->mmu.clear();
  this->decoder.pc = this->decoder.pc_start = 0;
  mmu.load_cart(cart);
}

u32 emulator_t::single_step() {

  u8 interrupt = (mmu.get(IoPorts::IE) & mmu.get(IoPorts::IF));

  if (interrupt) {
    // when an interrupt triggers we clear the IME
    // and the flag that triggered
    // and restart from a halting state
    cpu.halted = false;
    u8 handler = 0x40;
    auto set_interrupt = [this, &handler](int i) -> void {
      mmu.set(IoPorts::IF, mmu.get(IoPorts::IF) & ~(1 << (i - 1)));
      handler = 0x38 + 8 * i;
    };

    if (cpu.IME) {
      if (interrupt & 1)
        set_interrupt(1);
      else if (interrupt & 2)
        set_interrupt(2);
      else if (interrupt & 4)
        set_interrupt(3);
      else if (interrupt & 8)
        set_interrupt(4);
      else if (interrupt & 16)
        set_interrupt(5);
      else {
        return 0;
      } // something very strange happened here

      cpu.IME = 0;
      decoder.ii._push(decoder.pc);
      decoder.pc = handler;
      return 0; // this is so we have a debug step at the beginning of the
                // handler
    } else {
      // TODO: halt bug?
    }
  }

  joypad.tick();

  if (mmu.get(IoPorts::DMA)) {
    // technically this won't work if we transfer from 0
    // but that seems highly unlikely
    dma_transfer(&mmu, mmu.get(IoPorts::DMA));
    mmu.set(IoPorts::DMA, 0);
  }

  if (!cpu.halted) {
    decoder.decode();

    if (decoder.error) {
      log(decoder.pc_start, "decoder error", decoder.error);
      _stop();
      return 0;
    }
    if (decoder.ii.error) {
      log(decoder.pc_start, "runner error", decoder.ii.error);
      _stop();
      return 0;
    }
  }

  u32 dt = 16; // TODO: do timing based on actual instruction decodetime
  dt = 8;
  ppu.tick(dt);

  timer.tick(dt);
  return dt;
}

void emulator_t::step(i32 ticks) {
  // _runner.verbose_log = true;
  while (ticks > 0) {
    if (decoder.error || decoder.ii.error)
      break;

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
