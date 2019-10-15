#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <gl/gl.h>
#include <stdio.h>

void MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const char* message,
                 const void* userParam )
{
  printf("GL CALLBACK: %x type = 0x%x, severity = 0x%x, message = %s\n",
           // ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
         type,
           type, severity, message );
}

// During init, enable debug output

namespace glf {
  typedef void (*DEBUG_MSG_PROC)(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const char *message,
    const void *userParam);
  GLuint GL_DEBUG_OUTPUT = 0x92E0;
  enum ShaderType : GLenum { FRAGMENT_SHADER = 0x8B30, VERTEX_SHADER = 0x8B31 };
  enum BufferTarget : GLenum { ARRAY_BUFFER = 0x8892, ELEMENT_ARRAY_BUFFER = 0x8893 };
  enum DrawType : GLenum { STREAM_DRAW = 0x88E0, STATIC_DRAW = 0x88E4 };
  struct glShader { GLuint id; };

#define ALL_FUNCTIONS(F) \
  F(DebugMessageCallback, void (*)(DEBUG_MSG_PROC, void *)) \
  \
  F(CreateProgram, GLuint (*)()) \
  F(AttachShader, void (*)(GLuint, glShader))          \
  F(LinkProgram, void (*)(GLuint))          \
  F(UseProgram, void (*)(GLuint))          \
  F(DeleteProgram, void (*)(GLuint))          \
  F(CreateShader, glShader (*)(ShaderType)) \
  F(ShaderSource, void (*)(glShader, GLsizei, const char**, GLint*)) \
  F(CompileShader, void (*)(glShader)) \
  F(DeleteShader, void (*)(GLuint)) \
  \
  F(GenBuffers, void (*)(GLsizei, GLuint*)) \
  F(BindBuffer, void (*)(BufferTarget, GLuint)) \
  F(BufferData, void (*)(BufferTarget, GLsizei, const void*, DrawType)) \

#define GL_FUNC(NAME, TYPE) auto NAME = (TYPE)0; auto gl##NAME = (TYPE)0;
  ALL_FUNCTIONS(GL_FUNC)
  #undef GL_FUNC

  #define GL_FUNC(NAME, TYPE) gl##NAME = NAME = (TYPE)wglGetProcAddress("gl" #NAME); 
  void load_functions() {
    ALL_FUNCTIONS(GL_FUNC)
  }
  #undef GL_FUNC

}
