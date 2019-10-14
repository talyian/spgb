#include "base.hpp"
#include "platform.hh"

// functions we need to provide since WASM is a freestanding environment

size_t sslen(const char * s) {
  for(size_t i = 0;; i++, s++)
    if (!*s)
      return i;
}

extern "C" void *memset(void *dest, int c, size_t n) {
  u8 * d = (u8 *)dest;
  u8 * e = d + n;
  while(d < e) *d++ = c;
  return dest;
}

extern "C" void *memcpy(void *dest, const void *src, size_t n) {
  u8 * d = (u8 *)dest;
  u8 * s = (u8 *)src;
  u8 * e = d + n;
  while(d < e) *d++ = *s++;
  return dest;
}


extern "C" u8 * memory;
const size_t PAGE_SIZE = 64 * 1024;
u8 *wasm_allocate(size_t size) {
  log("wasm-allocate", (int)size);
  size_t page_start = __builtin_wasm_memory_grow(0, (size - 1) / PAGE_SIZE + 1);
  return memory + PAGE_SIZE * page_start;
}

void * operator new[](size_t size) {
  return wasm_allocate(size);
}
void * operator new(size_t size) {
  return wasm_allocate(size);
}
