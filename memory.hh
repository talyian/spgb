#include <cstdint>
#include <cstddef>
typedef uint8_t u8;
typedef uint16_t u16;

struct Memory {
  u8 buf[0x10000];
  bool running_bios;
  void load(u8 * src, size_t len, size_t start) {
    u8 * dest = buf + start;
    for(auto i=0u; i<len; i++) { *dest++ = *src++; }
  }
};
