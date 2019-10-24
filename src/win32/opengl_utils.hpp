#include "../base.hpp"
#include "../gb/graphics.hpp"

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
  enum PixelType : GLenum { UnsignedShort5551 = 0x8034, UnsignedShort1555R = 0x8366 };
  enum DebugSeverity : GLenum { HIGH = 0x9146, MEDIUM, LOW, NOTIFICATION=0x826B };
  struct glShader { GLuint id; };

void MessageCallback(
  GLenum source, GLenum type, GLuint id, GLenum severity,
  GLsizei length, const char* message, const void*) {
  // if (severity == DebugSeverity::NOTIFICATION) return;
  printf(
    "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %.*s\n",
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
  F(DeleteShader, void (*)(glShader)) \
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
// An 8-bit 216-color texture
struct Texture216 {
  GLuint id;
  Texture216(GLuint id) : id(id) { }    
  Texture216() {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  void setData(Pixel16 *data, u32 w, u32 h) {
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
        GL_RGBA, glf::PixelType::UnsignedShort1555R,
      data);
  }
  void setData(u8 * data, u32 w, u32 h) {
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RED, w, h, 0,
      // format/type is GL_RED/GL_UNSIGNED_BYTE to load single channel
      GL_RED, GL_UNSIGNED_BYTE, data);
  }
};

struct VBO {
  struct Vertex { f32 x, y, z; f32 u, v; };
  GLuint id = 0;
  VBO() { }
  VBO(Vertex * data, u32 count) { init(data, count); }
  void init(Vertex * data, u32 count) {
    glf::GenBuffers(1, &id);
    glf::BindBuffer(glf::ARRAY_BUFFER, id);
    glf::BufferData(
      glf::ARRAY_BUFFER,
      sizeof(Vertex) * count,
      data,
      glf::DrawType::STATIC_DRAW);
  }
};

struct Mesh {
  VBO vbo;
  Texture216 texture;
  
};
struct Shader {
  GLuint program = 0;
  
    const char *vs_source = R"STR(
#version 110
varying vec3 pos;
varying vec2 uv;
void main() { 
  vec4 vertex = ftransform();
  pos = vertex.xyz / gl_Vertex.w;
  uv = gl_MultiTexCoord0.xy;
  gl_Position = vec4(2.0 * pos - vec3(1.0), 1.0);
}
)STR";
    const char *fs_source = R"STR(
#version 110
varying vec3 pos;
varying vec2 uv;
uniform sampler2D tx_screen;
// Translate 8-bit 216-color-cube to RGB
void main() { 
  gl_FragColor = vec4(texture2D(tx_screen, uv).rgb, 1);
}
)STR";
  Shader() {
    program = glf::glCreateProgram();
    auto vertex_shader = glf::glCreateShader(glf::VERTEX_SHADER);
    glf::ShaderSource(vertex_shader, 1, &vs_source, 0);
    auto fragment_shader = glf::CreateShader(glf::FRAGMENT_SHADER);
    glf::ShaderSource(fragment_shader, 1, &fs_source, 0);
    glf::AttachShader(program, vertex_shader);
    glf::CompileShader(fragment_shader);
    glf::AttachShader(program, fragment_shader);
    glf::CompileShader(vertex_shader);
    glf::LinkProgram(program);
    glf::UseProgram(program);

    char info_log[1025];
    GLsizei size = 0;
    glf::GetProgramInfoLog(program, 1024, &size, info_log);
    if (size > 0) printf("GLSL Log: '%s'\n", info_log);
    glf::DeleteShader(vertex_shader);
    glf::DeleteShader(fragment_shader);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  void draw(VBO mesh, Texture216 texture) {
    glf::BindBuffer(glf::ARRAY_BUFFER, mesh.id);
    glVertexPointer(3, GL_FLOAT, sizeof(VBO::Vertex), (void*)0);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VBO::Vertex), (void*)12);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }
};
} // namespace glom
