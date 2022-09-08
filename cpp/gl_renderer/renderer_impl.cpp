#include "renderer_impl.h"
#include "atlas.h"
#include "shader.h"
#include "texture.h"
#include "vbo.h"
#include <glad/gl.h>
#include <iostream>

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

auto VS = R"(#version 110
attribute vec2 vPos;
attribute vec2 vTex;
attribute vec4 vCol;
void main()
{
    gl_Position = vec4(vPos, 0.0, 1.0);
}
)";

auto FS = R"(
#version 110
void main()
{
    gl_FragColor = vec4(1, 1, 1, 1.0);
}
)";

Vertex vertices[3] = {
    Vertex{
        .x = -0.5,
        .y = -0.5,
    },
    Vertex{
        .x = 0.5,
        .y = -0.5,
    },
    Vertex{
        .x = 0,
        .y = 0.5,
    },
};

uint32_t indices[3] = {
    0,
    1,
    2,
};

//
// Renderer
//
Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::initialize(const void *loadfunc) {
  gladLoadGL((GLADloadfunc)loadfunc);

  shader = Program::create(VS, FS);
  vbo = VBO::create(BUFFER_SIZE * sizeof(Vertex));
  vbo->update(vertices, sizeof(vertices), 3);
  ibo = IBO::create(BUFFER_SIZE * sizeof(uint32_t));
  ibo->update(indices, sizeof(indices));

  // vertex layout
  vbo->bind();
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)(sizeof(float) * 2));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex),
                        (void *)(sizeof(float) * 4));
  vbo->unbind();

  // init gl
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glDisable(GL_CULL_FACE);
  // glDisable(GL_DEPTH_TEST);
  // glEnable(GL_SCISSOR_TEST);
  // glEnable(GL_TEXTURE_2D);
}

void Renderer::begin(int width, int height, const UIColor32 &clr) {
  _width = width;
  _height = height;
  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);
  glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::flush() {
  shader->bind();
  atlas_texture->bind();
  vbo->bind();
  ibo->bind();

  ibo->draw();

  ibo->unbind();
  vbo->unbind();
  atlas_texture->unbind();
  shader->unbind();
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
  glScissor(rect.x, _height - (rect.y + rect.h), rect.w, rect.h);
}

void Renderer::push_quad(const UIRect &dst, const UIRect &src,
                         const UIColor32 &color) {
  // if (_triangle_count == BUFFER_SIZE) {
  //   flush();
  // }

  // int texvert_idx = _triangle_count * 4 * 2;
  // int color_idx = _triangle_count * 4 * 4;
  // int element_idx = _triangle_count * 4;
  // int index_idx = _triangle_count * 6;
  // _triangle_count++;

  // // update texture buffer
  // float x = src.x / (float)ATLAS_WIDTH;
  // float y = src.y / (float)ATLAS_HEIGHT;
  // float w = src.w / (float)ATLAS_WIDTH;
  // float h = src.h / (float)ATLAS_HEIGHT;
  // tex_buf[texvert_idx + 0] = x;
  // tex_buf[texvert_idx + 1] = y;
  // tex_buf[texvert_idx + 2] = x + w;
  // tex_buf[texvert_idx + 3] = y;
  // tex_buf[texvert_idx + 4] = x;
  // tex_buf[texvert_idx + 5] = y + h;
  // tex_buf[texvert_idx + 6] = x + w;
  // tex_buf[texvert_idx + 7] = y + h;

  // // update vertex buffer
  // vert_buf[texvert_idx + 0] = dst.x;
  // vert_buf[texvert_idx + 1] = dst.y;
  // vert_buf[texvert_idx + 2] = dst.x + dst.w;
  // vert_buf[texvert_idx + 3] = dst.y;
  // vert_buf[texvert_idx + 4] = dst.x;
  // vert_buf[texvert_idx + 5] = dst.y + dst.h;
  // vert_buf[texvert_idx + 6] = dst.x + dst.w;
  // vert_buf[texvert_idx + 7] = dst.y + dst.h;

  // // update color buffer
  // memcpy(color_buf + color_idx + 0, &color, 4);
  // memcpy(color_buf + color_idx + 4, &color, 4);
  // memcpy(color_buf + color_idx + 8, &color, 4);
  // memcpy(color_buf + color_idx + 12, &color, 4);

  // // update index buffer
  // index_buf[index_idx + 0] = element_idx + 0;
  // index_buf[index_idx + 1] = element_idx + 1;
  // index_buf[index_idx + 2] = element_idx + 2;
  // index_buf[index_idx + 3] = element_idx + 2;
  // index_buf[index_idx + 4] = element_idx + 3;
  // index_buf[index_idx + 5] = element_idx + 1;
}

void Renderer::push_matrix() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0f, _width, _height, 0.0f, -1.0f, +1.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

void Renderer::pop_matrix() {
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  assert(glGetError() == 0);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  assert(glGetError() == 0);
}
