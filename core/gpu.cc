#include "main.hh"

// socket info
#include "stdlib.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "netdb.h"
#include "memory.h"

// Pixel Processing Unit
struct PPU {
  Memory &mem;
  uint32_t clock = 0;
  bool bg_map;
  uint8_t &SCY; // screen-y
  uint8_t &SCX; // screen-x
  uint8_t &LY; // Line-Y

  enum {
    SCAN_OAM = 2,
    SCAN_VRAM = 3,
    HBLANK = 0,
    VBLANK = 1
  } state = SCAN_OAM;

  int sock = -1;
  GPU(Memory &mem) :
    mem(mem),
    SCY(mem[0xFF42]),
    SCX(mem[0xFF43]),
    LY(mem[0xFF44])
  {
    struct addrinfo hints, *servinfo = 0;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo("127.0.0.1", "33445", &hints, &servinfo)) exit(1);
    for(auto p = servinfo; p; p = p->ai_next) {
      sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (sock >= 0) {
        connect(sock, p->ai_addr, p->ai_addrlen);
        sendto(sock, "hello, world\n", 13, 0, p->ai_addr, p->ai_addrlen);
        break;
      }
    }

    auto urandom = fopen("/dev/urandom", "r");
    fread(&rhash, 4, 1, urandom);
    fclose(urandom);
    rhash = rhash % 10 + 2;
  }

  int frame = 0;
  uint rhash = 10;

  void RenderLine() { }
  void RenderScreen() { }
  void Render() {
    char msg[1 + 160 * 144];
    msg[0] = 'l';
    // strip pattern for test
    for(int y=0; y<144; y++)
      for(int x = 0; x < 160; x++)
        msg[1 + x + y * 160] = (x - y ) / rhash;

    // output tile map
    int len = 1 + 160 * 144;
    u8 * tile_map = &mem[0x9800];
    u8 * tile_data_0 = &mem[0x9000];
    //    u8 * tile_data_1 = &mem[0x8000];
    for(u16 y = SCY; y < SCY + 140; y++) {
      for(u16 x = 0; x < SCX + 160; x++) {
        u8 tile_y = y / 8;
        u8 tile_x = x / 8;
        u8 tile = tile_map[tile_y * 32 + tile_x];
        u8 px = x % 8;
        u8 py = y % 8;
        u8 pixel = tile_data_0[(int8_t)tile * 64 + px + py * 8];
        // u8 pixel = tile_data_1[tile * 64 + px + py * 8];
        msg[1 + (x - SCX) + (y - SCY) * 160] = pixel;
      }
    }
    for(int y = 0; y < 32; y++) {
      for(int x = 0; x < 32; x++) {
        u8 tile =tile_map[y * 32 + x];
        msg[1 + x + y * 160] = tile;
      }
    }
    send(sock, msg, len, 0);
  }

  void Step(uint32_t t) {
    clock += t;
    switch(state) {
    case SCAN_OAM:
      if (clock >= 80) {
        clock = 0;
        state = SCAN_VRAM;
      }; break;
    case SCAN_VRAM:
      if (clock >= 172) {
        clock = 0;
        state = HBLANK;
        RenderLine();
      }; break;
    case HBLANK:
      if (clock >= 204) {
        clock = 0;
        if (LY++ == 143) {
          state = VBLANK;
          RenderScreen();
        } else {
          state = SCAN_OAM;
        }
      }; break;
    case VBLANK:
      // In VBLANK
      if (clock >= 4560) {
        clock = 0;
        LY = 0;
        state = SCAN_OAM;
      }; break;
    }
  }
};
