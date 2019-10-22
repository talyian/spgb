#include "emulator.hpp"
#include "data/dmg_boot.hpp"

emulator_t::emulator_t(u8 *cart_data, u32 cart_len) {
  mmu.bios_rom = DMG_ROM_bin;
  load_cart(cart_data, cart_len);
}

emulator_t::emulator_t() : emulator_t(0, 0) {}

void emulator_t::load_cart(u8 *cart_data, u32 cart_len) {
  cart = Cart{cart_data, cart_len};
  cpu.clear();
  mmu.clear();
  timer.clear();
  ppu.clear();
  debug.state.type = Debugger::State::RUN;

  if (skip_bootrom) {
    mmu.BiosLock = 0x1;
    _executor.PC = _executor.PC_start = 0x100;
    cpu.registers.AF = 0x01B0;
    cpu.registers.BC = 0x0013;
    cpu.registers.DE = 0x00D8;
    cpu.registers.HL = 0x014D;
    cpu.registers.SP = 0xFFFE;
    mmu.set(io.IF, 0xE1);
  } else {
    _executor.PC = _executor.PC_start = 0;
    mmu.BiosLock = 0;
    // debug.state.type = Debugger::State::PAUSE;
  }
  mmu.load_cart(cart);
}

u32 emulator_t::single_step() {
  u32 dt = 16; // TODO: do timing based on actual instruction decodetime
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
      _executor._push(_executor.PC);
      _executor.PC = handler;

      return 0;  // we return to give the debugger a chance to latch.
    } else {
      // TODO: halt bug?
    }
  }

  if (!cpu.halted) {
    _executor.cycles = 0;
    _executor.decode();
    dt = _executor.cycles;
  }

  joypad.tick();
  if (mmu.get(IoPorts::DMA)) {
    // technically this won't work if we transfer from 0
    // but that seems highly unlikely
    dma_transfer(&mmu, mmu.get(IoPorts::DMA));
    mmu.set(IoPorts::DMA, 0);
  }
  audio.tick(dt);
  ppu.tick(dt);
  timer.tick(dt);

  return dt;
}

// calls single_step, but wraps in debug logic
// so returns either when ticks has elapsed or we're debugging
void emulator_t::step(i32 ticks) {
  // _runner.verbose_log = true;
  while (ticks > 0) {
    debug.step();
    if (_executor.error) break;
    // is_debugging means we don't run any code
    if (debug.state.type == Debugger::State::PAUSE) {
      _log("breakpoint");
      break;
    }
    ticks -= single_step();
  }
}
