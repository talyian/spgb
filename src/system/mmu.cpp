#include "mmu.hpp"
#include "ppu.hpp"

MemoryMapper::MemoryMapper(Cart &cart, Audio &audio, IoPorts &io) :
  cart(cart), audio(audio), io(io), BiosLock(io.data[0x50]) {
  set(0xFF42, 0);
  set(0xFF43, 0);
  set(0xFF44, 0);
}

void MemoryMapper::load_cart(const Cart &cart) {
  this->cart = cart;
}
