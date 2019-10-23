#include "../base.hpp"
#include "../emulator.hpp"
#include "../platform.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include <gl/gl.h>
#include "opengl_utils.hpp"
#include "audio.hpp"

extern "C" size_t sslen(const char* s) { return strlen(s); }

// Platform API: these functions are declared in platform.hpp and must
// be provided by the platform.
extern "C" {
  void _logf(double v) { printf("%f ", v); }
  void _logx8(u8 v) { printf("%02x ", v); }
  void _logx16(u16 v) { printf("%04x ", v); }
  void _logx32(u32 v) { printf("%04x ", v); }
  void _logs(const char* s, u32 len) { printf("%.*s ", len, s); }
  void _showlog() { printf("\n"); }
  void _stop() { exit(1); }
  void _logp(void* v) { printf("%p ", v); }
  void _serial_putc(u8 v) {
    static u64 sequence = 0;
    static char line[20];
    static u32 p = 0;
    auto flush = [&]() { printf("<<< %.*s\n", p, line); p = 0; };
    if (v == '\n' || p == 20) { flush(); } else { line[p++] = v; }
    sequence = sequence * 0x100 + v;
    if ((sequence & 0xFFFFFFFFFFFF) == *(u64 *)"dessaP\0") {
      flush();
      exit(0);
    }
    if ((sequence & 0xFFFFFFFFFFFF) == *(u64 *)"deliaF\0") {
      flush();
      exit(-1);
    }
  }
}

u32 round_po2(u32 v) {
  u32 p = 2;
  v--;
  while (v >>= 1) p <<= 1;
  return p;
}

// a global variable to hold current window/opengl/emulator context.
// Not hygienic, but it's an easy way to get access to these fields
// from inside WndProc or _push_frame.
struct Win32Emulator {
  HWND hwnd = 0;
  HGLRC gl = 0;
  HDC hdc = 0;
  emulator_t emu;
  glom::Shader * shader;
  int vbo_count = 0;

  RECT window_size {0, 0, 160 * 4, 144 * 4};
  enum SCREEN_STATE {
    MAIN = 0,
    TILES,
    BACKGROUND,
  } screen_state = MAIN;
  glom::Texture216 *texture_array;
  glom::Texture216 *screen_tex;
  glom::VBO *vbo_array = 0;
} win32_emulator;

u16 get_pc() {
  return win32_emulator.emu._executor.PC_start;
}
u64 get_monotonic_timer() {
  return win32_emulator.emu.timer.monotonic_t;
}
str get_symbol_name() {
  return "??";
}

// Main Window event handler.
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_QUIT:
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_SIZE: {
    u16 w = lParam;
    u16 h = lParam >> 16;
    win32_emulator.window_size.right = w;
    win32_emulator.window_size.bottom = h;
    glViewport(0, 0, w, h);

    {
      f32 w = 1;
      f32 h = 1;

      f32 d = 0.1;
      f32 u = win32_emulator.window_size.right / round_po2(16 * 9 * 2 + 1);
      f32 v = win32_emulator.window_size.bottom / round_po2(8 * 9 * 3 + 1);
      glom::VBO::Vertex vertices[] = {
        {0, 0, d, 0, v},
        {w, 0, d, u, v},
        {w, h, d, u, 0},
        {0, 0, d, 0, v},
        {w, h, d, u, 0},
        {0, h, d, 0, 0}
      };
      win32_emulator.vbo_array[1].init(vertices, sizeof(vertices) / sizeof(vertices[0]));
    }

    break;
  }
  case WM_KEYUP: {
    auto* jp = &win32_emulator.emu.joypad;
    switch (wParam) {
    case VK_RETURN: jp->button_up(Buttons::START); return 0;
    case VK_SHIFT:
    case VK_RSHIFT: jp->button_up(Buttons::SELECT); return 0;
    case 'J': jp->button_up(Buttons::B); return 0;
    case 'K': jp->button_up(Buttons::A); return 0;
    case 'W': jp->button_up(Buttons::UP); return 0;
    case 'S': jp->button_up(Buttons::DOWN); return 0;
    case 'A': jp->button_up(Buttons::LEFT); return 0;
    case 'D': jp->button_up(Buttons::RIGHT); return 0;

    }
  }
  case WM_KEYDOWN: {
    auto* jp = &win32_emulator.emu.joypad;
    switch (wParam) {
    case '1': win32_emulator.screen_state = Win32Emulator::MAIN; return 0;
    case '2': win32_emulator.screen_state = Win32Emulator::TILES; return 0;
    case '3': win32_emulator.screen_state = Win32Emulator::BACKGROUND; return 0;
    case '4': {
      // dump CGB palettes
      auto ppu = win32_emulator.emu.ppu;
      for (u32 i = 0; i < 8; i++) {
        printf("BG %d :", i);
        for (u32 p = 0; p < 4; p++) {
          auto bgpixel = ppu.Cgb.bg_palette.get_color(i, p);
          printf(" %04x", bgpixel);
        }
        printf("\n");
      }
      for (u32 i = 0; i < 8; i++) {
        printf("SP %d :", i);
        for (u32 p = 0; p < 4; p++) {
          auto bgpixel = ppu.Cgb.spr_palette.get_color(i, p);
          printf(" %04x", bgpixel);
        }
        printf("\n");
      }
      return 0;
    }
    case '5': {
      auto ppu = win32_emulator.emu.ppu;
      printf("BG tiles\n");
      // dump visible bg tiles
      for(int j = 0; j < 18; j++) {
        printf("# ");
        for(int i = 0; i < 20; i++) {
          printf("%02x ", ppu.VRAM[0x1800 + 32 * j + i]);
        }
        printf("\n| ");
        for(int i = 0; i < 20; i++) {
          printf("%02x ", ppu.VRAM2[0x1800 + 32 * j + i]);
        }
        printf("\n");
      }
      return 0;
    }
    case VK_RETURN: jp->button_down(Buttons::START); return 0;
    case VK_SHIFT:
    case VK_RSHIFT: jp->button_down(Buttons::SELECT); return 0;
    case 'J': jp->button_down(Buttons::B); return 0;
    case 'K': jp->button_down(Buttons::A); return 0;
    case 'W': jp->button_down(Buttons::UP); return 0;
    case 'S': jp->button_down(Buttons::DOWN); return 0;
    case 'A': jp->button_down(Buttons::LEFT); return 0;
    case 'D': jp->button_down(Buttons::RIGHT); return 0;
    case VK_SPACE:
      win32_emulator.emu.debug.state.type = Debugger::State::PAUSE;
      return 0;
    case VK_ESCAPE:
      if (win32_emulator.hwnd) DestroyWindow(win32_emulator.hwnd); return 0;
    }
  }
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}

int main(int argc, char** argv) {
  emulator_t& emu = win32_emulator.emu;

  // Select Cart from argv or open file Dialog box
  auto load_cart_file = [] (char* path, emulator_t* emu) -> void {
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "%s: file not found\n", path); exit(19); }
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, 0);
    u8* buf = new u8[len];
    fread(buf, 1, len, f);
    fclose(f);
    emu->load_cart(buf, len);
  };
  if (argc > 1) {
    load_cart_file(argv[1], &emu);
  }
  else {
    char filename[0x100] = { 0 };
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrInitialDir = NULL;
    if (!GetOpenFileName(&ofn)) {
      printf("no file specified\n");
      return 18;
    }
    load_cart_file(ofn.lpstrFile, &emu);
  }

  // init win32 window
  WNDCLASS wnd_class{ };
  wnd_class.hInstance = NULL;
  wnd_class.lpfnWndProc = WndProc;
  wnd_class.lpszClassName = "gbo";
  wnd_class.style = CS_HREDRAW | CS_VREDRAW;
  wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  if (!RegisterClass(&wnd_class)) {
    MessageBox(NULL, "RegisterClass failed", "Error", MB_OK);
    return 101;
  }

  struct v2i { u32 x, y; };
  RECT r = win32_emulator.window_size;
  AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
  HWND hwnd = win32_emulator.hwnd = CreateWindow(
    wnd_class.lpszClassName,
    "hi",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    r.right - r.left,
    r.bottom - r.top, NULL, NULL, NULL, NULL);

  MSG msg;

  // init WGL -- set hdc pixel format, then use wgl to create GL
  // context. wglCreateContext will fail without a compatible pixel
  // format.
  HDC hdc = win32_emulator.hdc = GetDC(hwnd);
  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 32;
  pfd.iLayerType = PFD_MAIN_PLANE;
  int px_format = ChoosePixelFormat(hdc, &pfd);
  if (px_format == 0) {
    printf("ChoosePixelFormat error\n");
    return __LINE__;
  }
  if (!SetPixelFormat(hdc, px_format, &pfd)) {
    printf("SetPixelFormat error\n");
    return __LINE__;
  }

  HGLRC gl = win32_emulator.gl = wglCreateContext(hdc);
  wglMakeCurrent(hdc, gl);
  auto version = glGetString(GL_VERSION);
  printf("OpenGL Version: %s\n", version);
  if (version[0] < '3') {
    printf("OpenGL Version Too Low\n");
    return 1;
  }

  glf::load_functions();
  glEnable(glf::GL_DEBUG_OUTPUT);
  glf::DebugMessageCallback(glf::MessageCallback, nullptr);

  glom::Shader shader {};
  win32_emulator.shader = &shader;
  win32_emulator.vbo_array = new glom::VBO[16];
  win32_emulator.texture_array = new glom::Texture216[16];
  win32_emulator.screen_tex = &win32_emulator.texture_array[0];
  {
    f32 w = 1.0f;
    f32 h = 1.0f;
    glom::VBO::Vertex vertices[] = {
      {0, 0, 0, 0, 1},
      {w, 0, 0,  1, 1},
      {w, h, 0,   1, 0},
      {0, 0, 0, 0, 1},
      {w, h, 0,   1, 0},
      {0, h, 0,  0, 0}
    };
    win32_emulator.vbo_array[0].init(vertices, sizeof(vertices)/sizeof(vertices[0]));
    win32_emulator.vbo_count++;
  }
  win32_emulator.screen_tex = &win32_emulator.texture_array[0];
  win32_emulator.vbo_count++; 

  audio_init();
  ShowWindow(hwnd, SW_RESTORE);

  if (emu.noswap) glDrawBuffer(GL_FRONT);
  
  char line[64] {0};
  if (argc > 1 && strstr(argv[1], "instr_timing")) {

  }
  if (argc > 1 && strstr(argv[1], "bgbtest")) {

  }
  if (argc > 1 && strstr(argv[1], "Kirby")) {
    // emu.debug.set_breakpoint(0x4B30);
  }
  if (argc > 1 && strstr(argv[1], "01-registers")) {
    // emu.debug.set_breakpoint(0xC2E0); // test_rw register
  }
  while (true) {
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) exit(0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      continue;
    }

    emu.debug.step();
    
    if (emu.debug.state.type == Debugger::State::PAUSE) {
      log("\x1b[1;31mime\x1b[0m", (u8)emu.cpu.IME, "interrupt", emu.mmu.get(0xFFFF), emu.mmu.get(0xFF0F));
      printf("Timer: CTL=%02x DIV=%02x TIMA=%02x, ticks=%d\n",
             emu.timer.Control, emu.timer.DIV, emu.timer.TIMA, (u16)emu.timer.monotonic_t);
      printf("PPU: STAT=%02x, LCDC=%02x LY=%02x LYC=%02x [%d%d%d]\n",
             emu.ppu.LcdStatus.v, emu.ppu.LcdControl,
             emu.ppu.LineY, emu.ppu.LineYMark,
             emu.ppu.LcdStatusMatch, emu.ppu.LcdStatusLastMatch, 
             emu.ppu.LcdStatusMatch - emu.ppu.LcdStatusLastMatch);
      auto rr = emu.cpu.registers;
      printf("AF   BC   DE   HL   SP   PC\n");
      printf("%04x %04x %04x %04x %04x %04x\n",
             (u16)rr.AF, (u16)rr.BC, (u16)rr.DE, (u16)rr.HL, (u16)rr.SP,
             emu._executor.PC);
      _log("\x1b[1;32m                  ");
      emu._printer.decode(emu._executor.PC);
      printf("\x1b[0m DEBUG %04x> ", emu._executor.PC);

      fgets(line, 63, stdin);
      for(int i = 0; i < 63; i++)
        if (!line[i]) break;
        else if (line[i] == '\n') { line[i] = 0; break; }
      
      if (!strcmp(line, "") || !strcmp(line, "s")) {
        emu.debug.state.type = Debugger::State::STEP;
        continue;
      }
      else if (!strcmp(line, "n")) {
        log("scanning to", emu._printer.PC);
        emu.debug.state.type = Debugger::State::RUN_TO;
        emu.debug.state.addr = emu._printer.PC;
        continue;
      }
      else if (!strcmp(line, "c")) {
        emu.debug.state.type = Debugger::State::RUN;
      }
      else if (!strcmp(line, "r")) {
        emu.debug.state.type = Debugger::State::RUN_TO_RET;
        emu.debug.state.call_depth = 1;
      }
      else if (!strcmp(line, "q")) {
        break;
      }
      else {
        continue;
      }
    }

    // 456 * 154 ticks is one emulator frame 
    // emu.step(456 * 154);
    // we need to singlestep here for debugging to work correctly
    emu.single_step();
  }
}

struct TileInfo {
  u8 * data = 0;
  TileInfo(u8 * data) : data(data) { }
  u8 get_pixel(u8 x, u8 y) {
    u8 a = data[2 * y];
    u8 b = data[2 * y + 1];
    u16 v = (a << 8) + b;
    u16 p = (v >> (7 - x)) & 0x0101;
    return (p >> 7) | p;
  }
};

u8 pal[4] = { 0, 36 + 12 + 2, 3 * 36  + 4 * 6 + 3, 215 };

u8 rgb2(u16 rgb);

void update_screen() {
  auto display = win32_emulator.emu.ppu.display;
  win32_emulator.screen_tex->setData(display, 160, 144);
}
//void update_tile_maps() {
//  auto &ppu = win32_emulator.emu.ppu;
//  auto TEXTURE_W = round_po2(16 * 9 * 2 + 1);
//  auto TEXTURE_H = round_po2(8 * 9 * 3 + 1);
//  static u8 * texture = new u8[TEXTURE_W * TEXTURE_H];
//  memset(texture, 0, TEXTURE_W * TEXTURE_H);
//
//  int BUTTON_W = 5;
//  for (u32 i = 0; i < 8; i++) {
//    for (u32 p = 0; p < 4; p++) {
//      auto bgpixel = ppu.Cgb.bg_palette.get_color(i, p);
//      auto sppixel = ppu.Cgb.spr_palette.get_color(i, p);
//
//      bgpixel = rgb2(bgpixel);
//      sppixel = rgb2(sppixel);
//      for(u32 x = 0; x < BUTTON_W; x++) 
//        for (u32 y = 0; y < BUTTON_W; y++) {
//          auto ty = i * (BUTTON_W) + 10 + y;
//          auto tx = p * (BUTTON_W) +(16 * 9 + 1) * 2 + 1 + x;
//          auto bb = bgpixel;
//          auto sp = sppixel;
//          if (x == 0 || y == 0) {
//            bb = 215;
//            sp = 215;
//          }
//          
//          texture[TEXTURE_W * ty + tx] = bb;
//          texture[TEXTURE_W * (ty + 40) + tx] = sp;
//        }
//    }
//  }
//  
//  for(u8 tile_bank = 0; tile_bank < 6; tile_bank++) {
//    u8 * bank;
//    if (tile_bank < 3) bank = &ppu.VRAM[0x800 * tile_bank];
//    else bank = &ppu.VRAM2[0x800 * tile_bank - 0x1800];
//    for(u8 tile = 0; tile < 16 * 8; tile++) {
//      auto tile_pos_x = tile % 16;
//      auto tile_pos_y = tile / 16;
//      TileInfo t(&bank[tile * 16]);
//      for(u8 ty = 0; ty < 8; ty++) {
//        for(u8 tx = 0; tx < 8; tx++) { 
//          u8 pixel = t.get_pixel(tx, ty);
//          // tileset position
//          u16 out_x = (tile_bank / 3) * (16 * 9 + 1);
//          u16 out_y = (tile_bank % 3) * (8 * 9 + 1);
//          // tile position
//          out_x += tile_pos_x * 9;
//          out_y += tile_pos_y * 9;
//          // pixel position
//          out_x += tx;
//          out_y += ty;
//          texture[TEXTURE_W * out_y + out_x] = pixel * 43;
//        }
//      }
//    }
//  }
//  win32_emulator.texture_array[1].setData(texture, TEXTURE_W, TEXTURE_H);
//}

void _push_frame(u32 category, u8* memory, u32 len) {
  audio_loop(1000.0 / 60);
  if (category - 0x100 < 3) {
    // show_tile_map(category, memory, len); 
  }
  else if (category == 0x300) {
    // update_tile_maps();
    update_screen();
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (win32_emulator.screen_state == Win32Emulator::MAIN) {
      win32_emulator.shader->draw(
        win32_emulator.vbo_array[0],
        win32_emulator.texture_array[0]);
    } else if (win32_emulator.screen_state == Win32Emulator::TILES) {
       win32_emulator.shader->draw(
         win32_emulator.vbo_array[0],
         win32_emulator.texture_array[0]);

      glClear(GL_DEPTH_BUFFER_BIT);

      win32_emulator.shader->draw(
        win32_emulator.vbo_array[1],
        win32_emulator.texture_array[1]);
    }

    if (win32_emulator.emu.noswap) 
      glFinish();
    else
      SwapBuffers(win32_emulator.hdc);
  }
}
