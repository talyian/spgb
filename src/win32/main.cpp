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

#define NOSWAP

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
  enum SCREEN_STATE {
    MAIN = 0,
    TILES,
    BACKGROUND,
  } screen_state = MAIN;
  glom::Texture216 *texture_array;
  glom::Texture216 *screen_tex;
  glom::VBO *vbo_array;
} win32_emulator;

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
    glViewport(0, 0, w, h);
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
    case VK_RETURN: jp->button_down(Buttons::START); return 0;
    case VK_SHIFT:
    case VK_RSHIFT: jp->button_down(Buttons::SELECT); return 0;
    case 'J': jp->button_down(Buttons::B); return 0;
    case 'K': jp->button_down(Buttons::A); return 0;
    case 'W': jp->button_down(Buttons::UP); return 0;
    case 'S': jp->button_down(Buttons::DOWN); return 0;
    case 'A': jp->button_down(Buttons::LEFT); return 0;
    case 'D': jp->button_down(Buttons::RIGHT); return 0;
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
  WNDCLASS wnd_class{ 0 };
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
  v2i viewport { 160 * 2, 144 * 2 };
  v2i screen { 160 * 2, 144 * 2 };
  RECT r = RECT();
  r.top = 0; r.left = 0;
  r.bottom = viewport.y; r.right = viewport.x;
  AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
  HWND hwnd = win32_emulator.hwnd = CreateWindow(
    wnd_class.lpszClassName,
    "hi",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    r.right - r.left,
    r.bottom - r.top, NULL, NULL, NULL, NULL);
  ShowWindow(hwnd, SW_RESTORE);

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
    f32 w = screen.x / (f32)viewport.x;
    f32 h = screen.y / (f32)viewport.y;
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
  for(u8 sprite_bank = 0; sprite_bank < 3; sprite_bank++){
    f32 scale = 2.0;
    f32 w = (1 + 16.0 * 9) / viewport.x * scale;
    f32 h = (1 +  8.0 * 9) / viewport.y * scale;
    f32 x = 0;
    f32 y = (sprite_bank * (8 * 9 + 2)) / (f32)viewport.y  * scale;
    f32 u = (1 + 16.0 * 9) / 256.0;
    f32 v = (1 + 8.0 * 9) / 256.0;
    glom::VBO::Vertex vertices[6] = {
      {x + 0, y + 0, 0, 0, v},
      {x + w, y + 0, 0, u, v},
      {x + w, y + h, 0, u, 0},
      {x + 0, y + 0, 0, 0, v},
      {x + w, y + h, 0, u, 0},
      {x + 0, y + h, 0, 0, 0}
    };
    win32_emulator.vbo_array[sprite_bank+1].init(vertices, 6);
    win32_emulator.vbo_count++;
  }
  #ifdef NOSWAP
  glDrawBuffer(GL_FRONT);
  #endif
  
  char line[64] {0};
  // emu.debug.name_function("main", 0xC300, 0xc315);
  // emu.debug.name_function("test_timer", 0xC318, 0xc345);
  if (argc > 1 && strstr(argv[1], "instr_timing"))
    emu.debug.set_breakpoint(0xC300);
  if (argc > 1 && strstr(argv[1], "bgbtest")) {
    // emu.debug.set_breakpoint(0x40); // vblank
    // emu.debug.set_breakpoint(0x40); // vblank
    // emu.debug.set_breakpoint(0x48); // lcdc
    // emu.debug.set_breakpoint(0x150); // entry
    // emu.debug.set_breakpoint(0x416);
    // emu.debug.set_breakpoint(0x1e7); // vblank
    // emu.debug.set_breakpoint(0x1e7); // vblank
    // emu.debug.set_breakpoint(0x1e1); // halt loop
    // emu.debug.set_breakpoint(0x219); // badfunction ?
  }
  if (argc > 1 && strstr(argv[1], "Kirby")) {
    emu.debug.set_breakpoint(0x4B30);
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
    emu.step(456 * 154);
    // emu.single_step();
  }
}

struct Sprite {
  u8 * data = 0;
  Sprite(u8 * data) : data(data) { }
  u8 get_pixel(u8 x, u8 y) {
    u8 a = data[2 * y];
    u8 b = data[2 * y + 1];
    u16 v = (a << 8) + b;
    u16 p = (v >> (7 - x)) & 0x0101;
    return (p >> 7) | p;
  }
};

u8 pal[4] = { 0, 36 + 12 + 2, 3 * 36  + 4 * 6 + 3, 215 };

void show_tile_map(u32 category, u8* memory, u32 len) {
    if (len != 0x800) { log("bad frame buffer"); _stop(); }
    u8 sprite_bank = category - 0x100;
    u32 buf_w = 256;
    u32 buf_h = 256;
    auto buf = new u8[buf_w * buf_h];
    for(u32 j=0; j < buf_w * buf_h; j++) {
      buf[j] = (4 * 36 + 3);
    }
    for(int tile = 0; tile < 16 * 8; tile++) {
      Sprite sprite(memory + 16 * tile);
      for(int y = 0; y < 8; y++)
        for(int x = 0; x < 8; x++) {
          u8 pixel = sprite.get_pixel(x, y);
          u16 px = (tile % 16) * 9 + x;
          u16 py = (tile / 16) * 9 + y;
          u16 idx = py * buf_w + px;
          if (idx < buf_w * buf_h)
            buf[idx] = pal[pixel % 3];
        }
    }
    win32_emulator.texture_array[sprite_bank+1].setData(buf, buf_w, buf_h);
    delete[] buf;    
}

void _push_frame(u32 category, u8* memory, u32 len) {
  if (category - 0x100 < 3) {
    show_tile_map(category, memory, len); 
  }
  else if (category == 0x300) {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (len != 160 * 144) { log("bad frame buffer"); _stop(); }
    win32_emulator.screen_tex->setData(memory, 160, 144);

    if (win32_emulator.screen_state == Win32Emulator::MAIN) {
      win32_emulator.shader->draw(
        win32_emulator.vbo_array[0],
        win32_emulator.texture_array[0]);
    } else if (win32_emulator.screen_state == Win32Emulator::TILES) {
      for(int i=0; i<3; i++) {
        win32_emulator.shader->draw(
          win32_emulator.vbo_array[1 + i],
          win32_emulator.texture_array[1 + i]);
      }
    }

    #ifdef NOSWAP
    glFinish();
    #else
    SwapBuffers(win32_emulator.hdc);
    #endif
  }
}
