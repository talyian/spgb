#include "../base.hpp"
#include "../gb/lib_gb.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include <gl/gl.h>
#include "opengl_utils.hpp"
#include "wasapi_audio.hpp"

// Platform API: these functions are declared in platform.hpp and must
// be provided by the platform.
extern "C" {
  void spgb_logf(double v) { printf("%f ", v); }
  void spgb_logx8(u8 v) { printf("%02x ", v); }
  void spgb_logx16(u16 v) { printf("%04x ", v); }
  void spgb_logx32(u32 v) { printf("%04x ", v); }
  void spgb_logs(const char* s, u32 len) { printf("%.*s ", len, s); }
  void spgb_showlog() { printf("\n"); }
  void spgb_stop() { exit(1); }
  void spgb_logp(void* v) { printf("%p ", v); }
  void spgb_serial_putc(u8 v) {
    static u64 sequence = 0;
    static char line[20];
    static u32 p = 0;
    auto flush = [&]() { printf("<<< %.*s\n", p, line); p = 0; };
    if (v == '\n' || p == 20) { flush(); }
    else { line[p++] = v; }
    sequence = sequence * 0x100 + v;
    if ((sequence & 0xFFFFFFFFFFFF) == *(u64*)"dessaP\0") {
      flush();
      exit(0);
    }
    if ((sequence & 0xFFFFFFFFFFFF) == *(u64*)"deliaF\0") {
      flush();
      exit(-1);
    }
  }
  u32 spgb_get_timestamp() {
    SYSTEMTIME sts;
    FILETIME fts;
    GetLocalTime(&sts);
    SystemTimeToFileTime(&sts, &fts);
    const u64 TICKS_PER_SEC = 10 * 1000 * 1000; // 
    const u64 UnixEpochOffsetSeconds = 11644473600LL;
    u64 ticks = fts.dwHighDateTime;
    ticks = (ticks << 32) | fts.dwLowDateTime;
    ticks = ticks / TICKS_PER_SEC - UnixEpochOffsetSeconds;
    return ticks;
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
  Emulator emu;
  glom::Shader* shader;
  int vbo_count = 0;

  RECT window_size{ 0, 0, 160 * 4, 144 * 4 };
  enum SCREEN_STATE {
    MAIN = 0,
    TILES,
    BACKGROUND,
  } screen_state = MAIN;
  glom::Texture216* texture_array;
  glom::Texture216* screen_tex;
  glom::VBO* vbo_array = 0;
} win32_emulator;

// Main Window event handler.
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  auto emu = win32_emulator.emu;
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
    switch (wParam) {
    case VK_RETURN: spgb_button_up(emu, Buttons::START); return 0;
    case VK_SHIFT:
    case VK_RSHIFT: spgb_button_up(emu, Buttons::SELECT); return 0;
    case 'J': spgb_button_up(emu, Buttons::B); return 0;
    case 'K': spgb_button_up(emu, Buttons::A); return 0;
    case 'W': spgb_button_up(emu, Buttons::UP); return 0;
    case 'S': spgb_button_up(emu, Buttons::DOWN); return 0;
    case 'A': spgb_button_up(emu, Buttons::LEFT); return 0;
    case 'D': spgb_button_up(emu, Buttons::RIGHT); return 0;

    }
  }
  case WM_KEYDOWN: {
    switch (wParam) {
    case '1': win32_emulator.screen_state = Win32Emulator::MAIN; return 0;
    case '2': win32_emulator.screen_state = Win32Emulator::TILES; return 0;
    case '3': win32_emulator.screen_state = Win32Emulator::BACKGROUND; return 0;
    case '4':
    case '5':
      break;
    case VK_RETURN: spgb_button_down(emu, Buttons::START); return 0;
    case VK_SHIFT:
    case VK_RSHIFT: spgb_button_down(emu, Buttons::SELECT); return 0;
    case 'J': spgb_button_down(emu, Buttons::B); return 0;
    case 'K': spgb_button_down(emu, Buttons::A); return 0;
    case 'W': spgb_button_down(emu, Buttons::UP); return 0;
    case 'S': spgb_button_down(emu, Buttons::DOWN); return 0;
    case 'A': spgb_button_down(emu, Buttons::LEFT); return 0;
    case 'D': spgb_button_down(emu, Buttons::RIGHT); return 0;
    case VK_SPACE:
      // win32_emulator.emu.debug.state.type = Debugger::State::PAUSE;
      return 0;
    case VK_ESCAPE:
      if (win32_emulator.hwnd) DestroyWindow(win32_emulator.hwnd); return 0;
    }
  }
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}

int main(int argc, char** argv) {
  Emulator emu = win32_emulator.emu = spgb_create_emulator();

  // Select Cart from argv or open file Dialog box
  auto load_cart_file = [](char* path, Emulator emu) -> void {
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "%s: file not found\n", path); exit(19); }
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, 0);
    u8* buf = new u8[len];
    fread(buf, 1, len, f);
    fclose(f);
    spgb_load_cart(emu, buf, len);
  };

  if (argc > 1) {
    load_cart_file(argv[1], emu);
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
    load_cart_file(ofn.lpstrFile, emu);
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
    "SPGB",
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

  glom::Shader shader{};
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
    win32_emulator.vbo_array[0].init(vertices, sizeof(vertices) / sizeof(vertices[0]));
    win32_emulator.vbo_count++;
  }
  win32_emulator.screen_tex = &win32_emulator.texture_array[0];
  win32_emulator.vbo_count++;

  ShowWindow(hwnd, SW_RESTORE);

  audio_init();

  // if (emu.noswap) glDrawBuffer(GL_FRONT);

  char line[64]{ 0 };
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

    for (int i = 0; i < 456; i++) // simulate one scanline
    // we need to singlestep here for debugging to work correctly
      spgb_step_instruction(emu);

    audio_loop(emu, 0);
  }
}

struct TileInfo {
  u8* data = 0;
  TileInfo(u8* data) : data(data) { }
  u8 get_pixel(u8 x, u8 y) {
    u8 a = data[2 * y];
    u8 b = data[2 * y + 1];
    u16 v = (a << 8) + b;
    u16 p = (v >> (7 - x)) & 0x0101;
    return (p >> 7) | p;
  }
};

u8 pal[4] = { 0, 36 + 12 + 2, 3 * 36 + 4 * 6 + 3, 215 };

u8 rgb2(u16 rgb);

void update_screen(Pixel16* display, u32 len) {
  win32_emulator.screen_tex->setData(display, 160, 144);
}

void spgb_push_frame(u32 category, u8* display, u32 len) {
  if (category - 0x100 < 3) {
    // show_tile_map(category, memory, len); 
  }
  else if (category == 0x300) {
    // update_tile_maps();
    update_screen((Pixel16*)display, len);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (win32_emulator.screen_state == Win32Emulator::MAIN) {
      win32_emulator.shader->draw(
        win32_emulator.vbo_array[0],
        win32_emulator.texture_array[0]);
    }
    else if (win32_emulator.screen_state == Win32Emulator::TILES) {
      win32_emulator.shader->draw(
        win32_emulator.vbo_array[0],
        win32_emulator.texture_array[0]);

      glClear(GL_DEPTH_BUFFER_BIT);

      win32_emulator.shader->draw(
        win32_emulator.vbo_array[1],
        win32_emulator.texture_array[1]);
    }

    // if (win32_emulator.emu.noswap) 
    //   glFinish();
    // else
    SwapBuffers(win32_emulator.hdc);
  }
}
