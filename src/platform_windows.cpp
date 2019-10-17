#include "base.hpp"
#include "emulator.hpp"
#include "platform.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include <gl/gl.h>
#include "win32_opengl.hpp"

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
    printf("Serial [%c]\n", v);
    sequence = sequence * 0x100 + v;
    if ((sequence & 0xFFFFFFFFFFFF) == *(u64 *)"dessaP\0\0")
      exit(0);
    if ((sequence & 0xFFFFFFFFFFFF) == *(u64 *)"deliaF\0\0")
      exit(-1);
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
  u32 display_texture = 0;
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

int main(int argc, char** argv) {
  emulator_t& emu = win32_emulator.emu;

  // Select Cart from argv or open file Dialog box
  auto load_cart_file = [] (char* path, emulator_t* emu) -> void {
    FILE* f = fopen(path, "r");
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
    OPENFILENAME ofn {0};
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
  if (version[0] < '3') {
    printf("OpenGL Version Too Low\n");
    return 1;
  }

  glf::load_functions();
  glEnable(glf::GL_DEBUG_OUTPUT);

  u32 display;

  auto program = glf::glCreateProgram();
  auto vertex_shader = glf::glCreateShader(glf::VERTEX_SHADER);
  const char * vs_source = R"STR(
void main() { gl_Position = ftransform(); }
)STR";
  glf::glShaderSource(vertex_shader, 1, &vs_source, 0);
  glf::glCompileShader(vertex_shader);
  auto fragment_shader = glf::glCreateShader(glf::VERTEX_SHADER);
  const char * fs_source = R"STR(
void main() { gl_FragColor = vec4(0.5, 1.0, 0.2, 1.0); }
)STR";
  glf::glShaderSource(fragment_shader, 1, &fs_source, 0);
  glf::glCompileShader(fragment_shader);
  glf::glAttachShader(program, vertex_shader);
  glf::glAttachShader(program, fragment_shader);
  glf::glLinkProgram(program);
  // glf::glUseProgram(program);

  // glDrawBuffer(GL_FRONT);
  GLuint vbo = 0;
  {
    f32 w = 1.0f;
    struct Vertex { f32 x, y, z; f32 u, v; } vertices[] = {
      {-w, -w, 0, 0, 1},
      {w, -w, 0,  1, 1},
      {w, w, 0,   1, 0},
      {-w, -w, 0, 0, 1},
      {w, w, 0,   1, 0},
      {-w, w, 0,  0, 0}
    };
    glf::GenBuffers(1, &vbo);
    glf::BindBuffer(glf::ARRAY_BUFFER, vbo);
    glf::BufferData(
      glf::ARRAY_BUFFER,
      sizeof(vertices),
      (const void*)vertices,
      glf::DrawType::STATIC_DRAW);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(vertices[0]), 0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(vertices[0]), (void*)12);
  }
  printf("vs: %d, fs: %d, error: %d\n", vertex_shader.id, fragment_shader.id, glGetError());
  
  glGenTextures(1, &display);
  glBindTexture(GL_TEXTURE_2D, display);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  win32_emulator.display_texture = display;

  char line[64] {0};
  // emu.debug.name_function("main", 0xC300, 0xc315);
  // emu.debug.name_function("test_timer", 0xC318, 0xc345);
  emu.debug.set_breakpoint(0xC300);
  while (true) {
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) exit(0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      continue;
    }

    emu.debug.step();
    
    if (emu.debug.state.type == Debugger::State::PAUSE) {
      log("ime", emu.cpu.IME, "interrupt", emu.mmu.get(0xFFFF), emu.mmu.get(0xFF0F));
      log("timer", emu.timer.Control, emu.timer.DIV, emu.timer.TIMA,
          (u32)emu.timer.counter_t,
          emu.timer.monoTIMA,
          (u32)emu.timer.monotonic_t);
      emu._runner.dump();
      emu.printer.pc = emu.decoder.pc; emu.printer.decode();
      printf("DEBUG %04x> ", emu.decoder.pc);

      fgets(line, 63, stdin);
      for(int i = 0; i < 63; i++)
        if (!line[i]) break;
        else if (line[i] == '\n') { line[i] = 0; break; }
      
      if (!strcmp(line, "") || !strcmp(line, "s")) {
        emu.debug.state.type = Debugger::State::STEP;
      }
      else if (!strcmp(line, "n")) {
        log("scanning to", emu.printer.pc);
        emu.debug.state.type = Debugger::State::RUN_TO;
        emu.debug.state.addr = emu.printer.pc;
      }
      else if (!strcmp(line, "c")) {
        emu.debug.state.type = Debugger::State::RUN;
      }
      else if (!strcmp(line, "r")) {
        emu.debug.state.type = Debugger::State::RUN;
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
    emu.single_step();
  }
}

void _push_frame(u32 category, u8* memory, u32 len) {
  if (category == 0x300) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, win32_emulator.display_texture);
    for (u32 i = 0; i < len; i++) {
      memory[i] = memory[i] == 0 ? 0 :
        memory[i] == 1 ? 0x60 :
        memory[i] == 2 ? 0x90 :
        0xFF;
    }
    glTexImage2D(
      GL_TEXTURE_2D, 0,
      GL_RGBA, // internal format
      160, 144,   //dimensions
      0 /*border*/,
      GL_GREEN /*format*/ ,
      GL_UNSIGNED_BYTE /*type*/,
      memory);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    // glFinish();
    SwapBuffers(win32_emulator.hdc);
  }
}
