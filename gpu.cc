
struct GPU {
  Memory &mem;
  uint32_t clock = 0;
  uint8_t line = 0;

  bool bg_map;
  uint8_t SCX = 0, SCY = 0;

  enum {
    SCAN_OAM = 2,
    SCAN_VRAM = 3,
    HBLANK = 0,
    VBLANK = 1
  } state = SCAN_OAM;

  GPU(Memory &mem) : mem(mem) { }

  int frame = 0;
  void Render() {
    // clear screen
    printf("\x1b[H");
    // printf("\033[H\033[J");
    // TODO: check bg_map
    u8 * tile_map = mem.buf + 0x9800;
    for(int y = 0; y < 32; y++) {
      for(int x = 0; x < 32; x++) {
        u8 tile =tile_map[y * 32 + x];
        if (tile == 0)
          printf(".");
        else
          printf("?");
      }
      printf("\n");
    }
  }

  void Step(uint32_t t) {
    clock += t;
    switch(state) {
    case SCAN_OAM: if (clock >= 80) { clock = 0; state = SCAN_VRAM; }; break;
    case SCAN_VRAM: if (clock >= 172) { clock = 0; state = HBLANK; }; break;
    case HBLANK: if (clock > 204) { clock = 0; state = VBLANK;}; break;
    case VBLANK:
      if (clock > 456) {
        clock = 0;
        if (++line > 153) {
          Render();
          line = 0; state = SCAN_OAM;
        }
      }; break;
    }
  }
};
