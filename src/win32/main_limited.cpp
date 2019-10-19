#include "../base.hpp"
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

typedef void * Emulator;
extern "C" Emulator get_emulator();
extern "C" void step_frame(Emulator emulator);
extern "C" void button_up(Emulator emulator, u16 button);
extern "C" void button_down(Emulator emulator, u16 button);
extern "C" void reset(Emulator emulator, u8 * cart, u32 cart_len);

Emulator g_emulator;
struct Rect { u32 w, h; } window_size;
HWND hwnd;
HDC hdc;
HGLRC gl;
glom::Texture216 * screen_tex;
glom::VBO * screen_vbo;
glom::Shader * shader;

// Main Window event handler.
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_QUIT:
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_SIZE: {
    u32 w = window_size.w = lParam;
    u32 h = window_size.h = lParam >> 16;
    glViewport(0, 0, w, h);
    break;
  }
  case WM_KEYUP: {
    switch (wParam) {
    case VK_RETURN: button_up(g_emulator, 7); return 0;
    case VK_SHIFT:
    case VK_RSHIFT: button_up(g_emulator, 6); return 0;
    case 'J': button_up(g_emulator, 5); return 0;
    case 'K': button_up(g_emulator, 4); return 0;
    case 'S': button_up(g_emulator, 3); return 0;
    case 'W': button_up(g_emulator, 2); return 0;
    case 'A': button_up(g_emulator, 1); return 0;
    case 'D': button_up(g_emulator, 0); return 0;
    }
  }
  case WM_KEYDOWN: {
    switch (wParam) {
    case VK_RETURN: button_down(g_emulator, 7); return 0;
    case VK_SHIFT:
    case VK_RSHIFT: button_down(g_emulator, 6); return 0;
    case 'J': button_down(g_emulator, 5); return 0;
    case 'K': button_down(g_emulator, 4); return 0;
    case 'S': button_down(g_emulator, 3); return 0;
    case 'W': button_down(g_emulator, 2); return 0;
    case 'A': button_down(g_emulator, 1); return 0;
    case 'D': button_down(g_emulator, 0); return 0;
    case VK_ESCAPE: if (hwnd) DestroyWindow(hwnd); return 0;
    }
  }
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}

int main(int argc, char** argv) {
  g_emulator = get_emulator();
  
  // Select Cart from argv or open file Dialog box
  auto load_cart_file = [] (char* path) -> void {
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr, "%s: file not found\n", path); exit(19); }
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, 0);
    u8* buf = new u8[len];
    fread(buf, 1, len, f);
    fclose(f);
    reset(g_emulator, buf, len);
  };
  if (argc > 1) {
    load_cart_file(argv[1]);
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
    load_cart_file(ofn.lpstrFile);
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
  hwnd = CreateWindow(
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
  hdc = GetDC(hwnd);
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

  gl = wglCreateContext(hdc);
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

  shader = new glom::Shader {};
  screen_tex = new glom::Texture216();
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
    screen_vbo = new glom::VBO(vertices, 6);
  }

  #ifdef NOSWAP
  glDrawBuffer(GL_FRONT);
  #endif
  
  while (true) {
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) exit(0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      continue;
    }
    step_frame(g_emulator);
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

void _push_frame(u32 category, u8* memory, u32 len) {
  if (category == 0x300) {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (len != 160 * 144) { log("bad frame buffer"); _stop(); }
    screen_tex->setData(memory, 160, 144);
    shader->draw(*screen_vbo, *screen_tex);

    #ifdef NOSWAP
    glFinish();
    #else
    SwapBuffers(win32_emulator.hdc);
    #endif
  }
}
