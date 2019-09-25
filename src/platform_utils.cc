#include "wasm_host.hpp"

i32 strlen(const char * s) {
  for(i32 i = 0;; i++, s++)
    if (!*s) return i;
}

void logs::_log(u8 v) { _logx8(v); }
void logs::_log(u16 v) { _logx16(v); }
void logs::_log(i32 v) { _logf(v); }
void logs::_log(double f) { _logf(f); }
void logs::_log(const char * s) { _logs(s, strlen(s)); }

extern "C" void *memcpy(void *dest, const void *src, size_t n) {
  u8 * d = (u8 *)dest;
  u8 * s = (u8 *)src;
  for(size_t i = 0; i<n; i++)    *d++ = *s++;
  return dest;
}

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

extern "C" void load_rom(u8 * data, u32 len, int option) {
  
}

const int w = 160, h = 144;
u32 frame_buffer [w * h];
extern "C" u32* get_ppu_frame(int ticks_elapsed) {
  for(int i=0; i< w * h; i++) { frame_buffer[i] = i; }
  return frame_buffer;
};

u8 extern_memory[0x10000];

extern"C" u8 * get_rom_memory() {
  return extern_memory;
}
