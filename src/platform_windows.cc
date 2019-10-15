#include "base.hpp"
#include "emulator.hpp"
#include "platform.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include <gl/gl.h>

extern "C" size_t sslen(const char* s) { return strlen(s); }

// imports
extern "C" {
  void _logf(double v) { printf("%f ", v); }
  void _logx8(u8 v) { printf("%02x ", v); }
  void _logx16(u16 v) { printf("%04x ", v); }
  void _logx32(u32 v) { printf("%04x ", v); }
  void _logs(const char* s, u32 len) { printf("%.*s ", len, s); }
  void _showlog() { printf("\n"); }
  void _stop() { exit(1); }
  void _logp(void* v) { printf("%p ", v); }
  void _serial_putc(char c) {
  }
}

struct Win32Emulator {
  HWND hwnd = 0;
  HGLRC gl = 0;
  HDC hdc = 0;
  emulator_t emu;
  u32 display_texture = 0;
} win32_emulator;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_QUIT:
    printf("???quit\n");
    return 0;
  case WM_DESTROY:
    printf("destroyed\n");
    PostQuitMessage(0);
    return 0;
  case WM_SIZE: {
    u16 w = lParam;
    u16 h = lParam >> 16;
    glViewport(0, 0, w, h);
    break;
  }
  case WM_PAINT:
    break;
  case WM_KEYUP: {
    auto* jp = &win32_emulator.emu.joypad;
    switch (wParam) {
    case VK_RETURN: jp->button_up(Buttons::START); return 0;
    case VK_RSHIFT: jp->button_up(Buttons::SELECT); return 0;
    case 'J': jp->button_up(Buttons::B); return 0;
    case 'K': jp->button_up(Buttons::A); return 0;
    case 'W': jp->button_up(Buttons::UP); return 0;
    case 'S': jp->button_up(Buttons::DOWN); return 0;
    case 'A': jp->button_up(Buttons::LEFT); return 0;
    case 'D': jp->button_up(Buttons::RIGHT); return 0;
    case VK_ESCAPE:
      DestroyWindow(hwnd); return 0;
    }
  }
  case WM_KEYDOWN: {
    auto* jp = &win32_emulator.emu.joypad;
    switch (wParam) {
    case VK_RETURN: jp->button_down(Buttons::START); return 0;
    case VK_RSHIFT: jp->button_down(Buttons::SELECT); return 0;
    case 'J': jp->button_down(Buttons::B); return 0;
    case 'K': jp->button_down(Buttons::A); return 0;
    case 'W': jp->button_down(Buttons::UP); return 0;
    case 'S': jp->button_down(Buttons::DOWN); return 0;
    case 'A': jp->button_down(Buttons::LEFT); return 0;
    case 'D': jp->button_down(Buttons::RIGHT); return 0;
    case VK_ESCAPE:
      DestroyWindow(hwnd); return 0;
    }
  }
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}

u8* display_memory = 0;
u32 display_memory_len = 0;

// Reads a cart file from a path
void load_cart_file(char* path, emulator_t* emu) {
  FILE* f = fopen(path, "r");
  if (!f) { fprintf(stderr, "%s: file not found\n", path); exit(19); }
  fseek(f, 0, SEEK_END);
  size_t len = ftell(f);
  fseek(f, 0, 0);
  u8* buf = new u8[len];
  fread(buf, 1, len, f);
  fclose(f);
  emu->load_cart(buf, len);
}

int main(int argc, char** argv) {
  // init emulator 
  emulator_t& emu = win32_emulator.emu;
  u8 last_serial_cursor = 0, first_serial = 1;

  // Select Cart
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
    ofn.nMaxFile = 0x100;
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

  RECT r = RECT();
  r.top = 0; r.left = 0;
  r.bottom = 144 * 2; r.right = 160 * 2;
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

  u32& display = win32_emulator.display_texture;
  glGenTextures(1, &display);
  glBindTexture(GL_TEXTURE_2D, display);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  long long int frames = 1;
  while (true) {
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) exit(0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      continue;
    }

    for (int i = 0; i < 4000000 / 60; i++)
      emu.single_step();
  }
}

void _push_frame(u32 category, u8* memory, u32 len) {
  if (category == 0x300) {
    display_memory = memory;
    display_memory_len = len;

    glClearColor(1.0f, 0.5f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, win32_emulator.display_texture);
    for (u32 i = 0; i < len; i++) {
      memory[i] = memory[i] == 0 ? 0 :
        memory[i] == 1 ? 0x60 :
        memory[i] == 2 ? 0x90 :
        0xFF;
    }
    glTexImage2D(
      GL_TEXTURE_2D, 0,
      GL_R3_G3_B2, // internal format
      160, 144, //dimensions
      0 /*border*/, GL_RED /*format*/, GL_UNSIGNED_BYTE /*type*/,
      memory);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
    auto w = 1.0f;
    glTexCoord2f(0, 1);
    glVertex3f(-w, -w, 0);
    glTexCoord2f(1, 1);
    glVertex3f(w, -w, 0);
    glTexCoord2f(1, 0);
    glVertex3f(w, w, 0);

    glTexCoord2f(0, 1);
    glVertex3f(-w, -w, 0);
    glTexCoord2f(1, 0);
    glVertex3f(w, w, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-w, w, 0);
    glEnd();

    if (win32_emulator.hdc)
      SwapBuffers(win32_emulator.hdc);
  }
}

//   emulator_t emu {};

//   if (argc > 1) {
//     FILE * f = fopen(argv[1], "r");
//     if (!f) { fprintf(stderr, "%s: file not found\n", argv[1]); exit(19); }
//     fseek(f,0,SEEK_END);
//     size_t len = ftell(f);
//     fseek(f,0,0);
//     u8 * buf = new u8[len];
//     fread(buf, 1, len, f);
//     fclose(f);
//     emu.load_cart(buf, len);
//   } else {
//     printf("no file specified\n");
//     return 18;
//   }
//   // emu.set_breakpoint(0x40); // vblank interrupt
//   // emu.set_breakpoint(0xFF80); // high memory DMA loading thunk
//   // emu.debug.is_debugging = true;
//   // emu.debug.set_breakpoint(0xC66F); // Blargg 07 issue debugging
//   // emu.debug.set_breakpoint(0xC681); // Blargg 07 issue debugging

//   // emu.debug.set_breakpoint(0xc2b4); // Blargg 02 main function "EI"
//   // emu.debug.set_breakpoint(0xc316); // Blargg 02 "timer doesn't work"
//   // emu.debug.set_breakpoint(0xC2E7); // Blargg combined "03 test strange"
//   u8 last_serial_cursor = 0, first_serial = 1;
//   char line[64] {0};
//   while(true) {
//     emu.debug.step();

//     if (emu.debug.is_debugging) {
//       log("ime", emu.cpu.IME, "interrupt", emu.mmu.get(0xFFFF),
//       emu.mmu.get(0xFF0F)); log("timer", emu.timer.Control, emu.timer.DIV,
//       emu.timer.TIMA,
//           (u32)emu.timer.counter_t,
//           emu.timer.monoTIMA,
//           (u32)emu.timer.monotonic_t);
//       emu._runner.dump();
//       emu.printer.pc = emu.decoder.pc; emu.printer.decode();
//       printf("DEBUG %04x> ", emu.decoder.pc);

//       fgets(line, 63, stdin);
//       for(int i = 0; i < 63; i++)
//         if (!line[i]) break;
//         else if (line[i] == '\n') { line[i] = 0; break; }

//       if (!strcmp(line, "") || !strcmp(line, "s")) {
//         emu.debug.is_stepping = true;
//         emu.debug.is_debugging = false;
//       }
//       else if (!strcmp(line, "n")) {
//         log("scanning to", emu.printer.pc);
//         emu.debug.run_to_target = emu.printer.pc;
//         emu.debug.is_debugging = false;
//       }
//       else if (!strcmp(line, "c")) {
//         emu.debug.is_debugging = false;
//       }
//       else if (!strcmp(line, "q")) {
//         break;
//       }
//       else {
//         continue;
//       }
//     }

//     emu.single_step();

//     auto &serial = emu.cpu.serial;
//     // if (serial.pos >= last_serial_cursor && serial.out_buf[serial.pos] ==
//     '\n') { if (last_serial_cursor < serial.pos) {
//       // printf("\x1b[1;31m");
//       if (first_serial) { printf("Serial: "); first_serial = 0; }
//       // blargg tests
//       for(; last_serial_cursor < serial.pos; last_serial_cursor++) {
//         u8 c = serial.out_buf[last_serial_cursor];
//         if (c >= 0x20 || c == '\n')
//           putchar(c);
//         else
//           printf("\\x%02x", c);
//         if (c == '\n') {
//           first_serial = 1;
//         }
//       }
//       // printf("\x1b[0m");
//       serial.out_buf[255] = 0;
//       if (strstr((const char *)serial.out_buf, "Passed") ||
//           strstr((const char *)serial.out_buf, "Failed")) {
//         exit(0);
//       }
//     }
//   }
// }
