#include "wasm_host.hpp"

void logs::_log(u8 v) { _logx8(v); }
void logs::_log(u16 v) { _logx16(v); }
void logs::_log(u32 v) { _logx32(v); }
void logs::_log(i32 v) { _logf(v); }
void logs::_log(double f) { _logf(f); }
void logs::_log(const char * s) { _logs(s, strlen(s)); }
void logs::_log(void * s) { _logp(s); }

#ifdef WASM
size_t strlen(const char * s) {
  for(i32 i = 0;; i++, s++)
    if (!*s) return i;
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

#endif

struct CartHeader {
  u8 entry_point[4];
  u8 nintendo_logo[0x30];
  u8 title[15];
  u8 is_color_gb; // 0x80 for color
  u8 licensee[2];
  u8 is_super_gb; // 0x3 for super, 0x0 for game boy
  u8 cartridge_type;
  u8 rom_size;
  u8 ram_size;
  u8 destination_code;
  u8 licensee_code;
  u8 mask_rom_version;
  u8 complement_check;
  u8 checksum[2];
};
