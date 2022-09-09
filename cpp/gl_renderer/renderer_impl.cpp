#include "renderer_impl.h"
#include "atlas.h"
#include "shader.h"
#include "texture.h"
#include "vbo.h"
#include <glad/gl.h>
#include <iostream>

const auto MAX_QUADS = 12800;

auto VS = R"(#version 110
attribute vec2 vPos;
attribute vec2 vTex;
attribute vec4 vCol;
varying vec4 user_Color;
uniform mat4 M;
void main()
{
    gl_Position = M * vec4(vPos, 0.0, 1.0);
    user_Color = vCol;
}
)";

auto FS = R"(
#version 110
varying vec4 user_Color;
void main()
{
    gl_FragColor = user_Color;
}
)";

void errorCheck() {
  GLenum err;
  while (true) {
    auto err = glGetError();
    if (err == GL_NO_ERROR) {
      break;
    }

    switch (err) {
    case GL_INVALID_ENUM:
      std::cerr << "GL_INVALID_ENUM" << std::endl;
      break;
    case GL_INVALID_VALUE:
      std::cerr << "GL_INVALID_VALUE" << std::endl;
      break;
    case GL_INVALID_OPERATION:
      std::cerr << "GL_INVALID_OPERATION" << std::endl;
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
      break;
    case GL_OUT_OF_MEMORY:
      std::cerr << "GL_OUT_OF_MEMORY" << std::endl;
      break;
    case GL_STACK_UNDERFLOW:
      std::cerr << "GL_STACK_UNDERFLOW" << std::endl;
      break;
    case GL_STACK_OVERFLOW:
      std::cerr << "GL_STACK_OVERFLOW" << std::endl;
      break;
    }
  }
}

// Vertex vertices[3] = {
//     Vertex{
//         .x = 10,
//         .y = 10,
//     },
//     Vertex{
//         .x = 100,
//         .y = 10,
//     },
//     Vertex{
//         .x = 10,
//         .y = 100,
//     },
// };

// uint32_t indices[3] = {
//     0,
//     1,
//     2,
// };

//
// Renderer
//
Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::initialize(const void *loadfunc) {
  gladLoadGL((GLADloadfunc)loadfunc);

  shader = Program::create(VS, FS);
  vao = VAO::create();
  vao->vbo = VBO::create(MAX_QUADS * 6 * sizeof(Vertex));
  vao->ibo = IBO::create(MAX_QUADS * 4 * sizeof(uint32_t));

  // https://www.khronos.org/opengl/wiki/Vertex_Specification
  vao->bind();
  vao->vbo->bind();
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, u));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                        (void *)offsetof(Vertex, color));
  vao->ibo->bind();
  vao->unbind();
  vao->vbo->unbind();
  vao->ibo->unbind();

  // init gl
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glDisable(GL_CULL_FACE);
  // glDisable(GL_DEPTH_TEST);
  // glEnable(GL_SCISSOR_TEST);
  // glEnable(GL_TEXTURE_2D);
}

void Renderer::begin(int width, int height, const UIColor32 &clr) {
  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);
  glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
  glClear(GL_COLOR_BUFFER_BIT);

  // glOrtho(0.0f, _width, _height, 0.0f, -1.0f, +1.0f);
  _matrix[0] = 2.0f / width;
  _matrix[3] = -1;
  _matrix[5] = 2.0f / -height;
  _matrix[7] = 1;
}

void Renderer::flush() {
  if (!_vertices.empty() && !_indices.empty()) {
    // update
    vao->vbo->update(_vertices);
    vao->ibo->update(_indices);
    _vertices.clear();
    _indices.clear();

    // draw
    shader->bind();
    shader->set_uniform_matrix("M", _matrix, true);
    atlas_texture->bind();
    vao->draw();
    atlas_texture->unbind();
    shader->unbind();
  }
}

void Renderer::draw_rect(const UIRect &rect, const UIColor32 &color) {
  push_quad(rect, *((UIRect *)&atlas[ATLAS_WHITE]), color);
}

void Renderer::draw_text(const uint8_t *begin, const uint8_t *end,
                         const UIVec2 &pos, const UIColor32 &color) {
  UIRect dst = {pos.x, pos.y, 0, 0};
  for (auto *p = begin; p != end; p++) {
    if ((*p & 0xc0) == 0x80) {
      continue;
    }
    int chr = std::min((uint8_t)*p, (uint8_t)127);
    UIRect src = *((UIRect *)&atlas[ATLAS_FONT + chr]);
    dst.w = src.w;
    dst.h = src.h;
    push_quad(dst, src, color);
    dst.x += dst.w;
  }
}

void Renderer::draw_icon(int id, const UIRect &rect, const UIColor32 &color) {
  UIRect src = *((UIRect *)&atlas[id]);
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  push_quad(UIRect(x, y, src.w, src.h), src, color);
}

void Renderer::set_clip_rect(const UIRect &rect) {
  flush();
  // std::cerr << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h
  //           << std::endl;
  // glScissor(rect.x, _height - (rect.y + rect.h), rect.w, rect.h);
}

void Renderer::push_quad(const UIRect &quad, const UIRect &glyph,
                         const UIColor32 &color) {

  UIColor32 rgba = {
      color.r,
      color.g,
      color.b,
      color.a,
  };

  float x = glyph.x / (float)ATLAS_WIDTH;
  float y = glyph.y / (float)ATLAS_HEIGHT;
  float w = glyph.w / (float)ATLAS_WIDTH;
  float h = glyph.h / (float)ATLAS_HEIGHT;
  auto i = _vertices.size();

  // 0 - 1
  // |   |
  // 2 - 3
  //
  // 0,1,2
  // 2,3,1

  _vertices.push_back(Vertex{
      .x = (float)quad.x,
      .y = (float)quad.y,
      .u = x,
      .v = y,
      .color = rgba,
  });

  _vertices.push_back(Vertex{
      .x = (float)(quad.x + quad.w),
      .y = (float)quad.y,
      .u = x + w,
      .v = y,
      .color = rgba,
  });

  _vertices.push_back(Vertex{
      .x = (float)quad.x,
      .y = (float)(quad.y + quad.h),
      .u = x,
      .v = y + h,
      .color = rgba,
  });

  _vertices.push_back(Vertex{
      .x = (float)(quad.x + quad.w),
      .y = (float)(quad.y + quad.h),
      .u = x + w,
      .v = y + h,
      .color = rgba,
  });

  // triangle 0
  _indices.push_back(i + 0);
  _indices.push_back(i + 1);
  _indices.push_back(i + 2);
  // triangle 1
  _indices.push_back(i + 2);
  _indices.push_back(i + 3);
  _indices.push_back(i + 1);
}
