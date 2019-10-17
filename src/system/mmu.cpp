#include "system/mmu.hpp"
#include "system/ppu.hpp"

MemoryMapper::MemoryMapper(Cart &cart, IoPorts &io) :
  cart(cart), io(io), BiosLock(io.data[0x50]) {
  set(0xFF42, 0);
  set(0xFF43, 0);
  set(0xFF44, 0);
}

void MemoryMapper::load_cart(const Cart &cart) {
  this->cart = cart;
}
