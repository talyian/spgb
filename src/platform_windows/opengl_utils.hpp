#include "../base.hpp"

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <gl/gl.h>
#include <stdio.h>

// GLF: lightweight wrapper around WGL
// - provides an function loader glf::load_functions(); call this after creating a context.
// - defines a slightly typed version of the openGL API
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
  enum ShaderIV : GLenum { COMPILE_STATUS = 0x8B81, VALIDATE_STATUS= 0x8B83, INFO_LOG_LENGTH = 0x8B84 };
  enum DebugType : GLenum { ERR = 0x824C, DEPRECATED = 0x824D, UNDEFINED_BEHAVIOR = 0x824E, PERFORMANCE = 0x8250 };
  struct glShader { GLuint id; };

void MessageCallback(
  GLenum source, GLenum type, GLuint id, GLenum severity,
  GLsizei length, const char* message, const void*) {
  printf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %.*s\n",
         ( type == DebugType::ERR ? "** GL ERROR **" : "" ), type, severity, length, message);
}

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
  F(GetShaderiv, void(*)(glShader, GLenum, GLint *)) \
  F(GetShaderInfoLog, void (*)(glShader, GLsizei, GLsizei *, char*))  \
  F(GetProgramInfoLog, void (*)(GLuint, GLsizei, GLsizei *, char*))  \
  F(VertexAttribPointer, void (*)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)); \
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


// GLOM: Object Model for GL resources
namespace glom {
  struct Vertex { f32 x, y, z, u, v; };
  struct TriangleMesh {
    Vertex * data;
    u32 vertex_count;
  };

  // An 8-bit 216-color texture
  struct Texture216 {
    GLuint id;
    u32 len = 0;
    Texture216(GLuint id) : id(id) { }    
    Texture216() {
      glGenTextures(1, &id);
      glBindTexture(GL_TEXTURE_2D, id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    void setData(u8 * data, u32 w, u32 h) {
      glBindTexture(GL_TEXTURE_2D, id);
      glTexImage2D(
        GL_TEXTURE_2D, 0,
        GL_RED,
        w, h, 0,
        // format/type is GL_RED/GL_UNSIGNED_BYTE to load single channel 
        GL_RED, GL_UNSIGNED_BYTE, data);
    }
  };
}
