#pragma once
#include <UIRenderFrame.h>
#include <iostream>

const auto BUFFER_SIZE = 16384;
class Renderer {
  int _width = 800;
  int _height = 600;
  int _triangle_count = 0;
  GLuint _id = 0;

  GLfloat tex_buf[BUFFER_SIZE];
  GLfloat vert_buf[BUFFER_SIZE];
  GLubyte color_buf[BUFFER_SIZE];
  GLuint index_buf[BUFFER_SIZE];

public:
  Renderer() {}

  ~Renderer() { glDeleteTextures(1, &_id); }

  void load_atlas(int w, int h, const unsigned char *data) {
    // init gl
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // texture
    glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA,
                 GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    assert(glGetError() == 0);
  }

  void begin(int width, int height, const UIColor32 &clr) {
    assert(_triangle_count == 0);
    _width = width;
    _height = height;
    glViewport(0, 0, width, height);
    assert(glGetError() == 0);
    glScissor(0, 0, width, height);
    assert(glGetError() == 0);
    glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
    assert(glGetError() == 0);
    glClear(GL_COLOR_BUFFER_BIT);
    assert(glGetError() == 0);
  }

  void flush() {
    if (_triangle_count) {
      glViewport(0, 0, _width, _height);
      assert(glGetError() == 0);
      push_matrix();
      glTexCoordPointer(2, GL_FLOAT, 0, tex_buf);
      assert(glGetError() == 0);
      glVertexPointer(2, GL_FLOAT, 0, vert_buf);
      assert(glGetError() == 0);
      glColorPointer(4, GL_UNSIGNED_BYTE, 0, color_buf);
      assert(glGetError() == 0);
      glDrawElements(GL_TRIANGLES, _triangle_count * 6, GL_UNSIGNED_INT,
                     index_buf);
      assert(glGetError() == 0);
      pop_matrix();
    }
    _triangle_count = 0;
  }

  void draw_rect(const UIRect &rect, const UIColor32 &color) {
    push_quad(rect, *((UIRect *)&atlas[ATLAS_WHITE]), color);
  }

  void draw_text(const uint8_t *begin, const uint8_t *end, const UIVec2 &pos,
                 const UIColor32 &color) {
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

  void draw_icon(int id, const UIRect &rect, const UIColor32 &color) {
    UIRect src = *((UIRect *)&atlas[id]);
    int x = rect.x + (rect.w - src.w) / 2;
    int y = rect.y + (rect.h - src.h) / 2;
    push_quad(UIRect(x, y, src.w, src.h), src, color);
  }

  void set_clip_rect(const UIRect &rect) {
    flush();
    // std::cerr << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h
    //           << std::endl;
    glScissor(rect.x, _height - (rect.y + rect.h), rect.w, rect.h);
    assert(glGetError() == 0);
  }

private:
  void push_quad(const UIRect &dst, const UIRect &src, const UIColor32 &color) {
    if (_triangle_count == BUFFER_SIZE) {
      flush();
    }

    int texvert_idx = _triangle_count * 4 * 2;
    int color_idx = _triangle_count * 4 * 4;
    int element_idx = _triangle_count * 4;
    int index_idx = _triangle_count * 6;
    _triangle_count++;

    // update texture buffer
    float x = src.x / (float)ATLAS_WIDTH;
    float y = src.y / (float)ATLAS_HEIGHT;
    float w = src.w / (float)ATLAS_WIDTH;
    float h = src.h / (float)ATLAS_HEIGHT;
    tex_buf[texvert_idx + 0] = x;
    tex_buf[texvert_idx + 1] = y;
    tex_buf[texvert_idx + 2] = x + w;
    tex_buf[texvert_idx + 3] = y;
    tex_buf[texvert_idx + 4] = x;
    tex_buf[texvert_idx + 5] = y + h;
    tex_buf[texvert_idx + 6] = x + w;
    tex_buf[texvert_idx + 7] = y + h;

    // update vertex buffer
    vert_buf[texvert_idx + 0] = dst.x;
    vert_buf[texvert_idx + 1] = dst.y;
    vert_buf[texvert_idx + 2] = dst.x + dst.w;
    vert_buf[texvert_idx + 3] = dst.y;
    vert_buf[texvert_idx + 4] = dst.x;
    vert_buf[texvert_idx + 5] = dst.y + dst.h;
    vert_buf[texvert_idx + 6] = dst.x + dst.w;
    vert_buf[texvert_idx + 7] = dst.y + dst.h;

    // update color buffer
    memcpy(color_buf + color_idx + 0, &color, 4);
    memcpy(color_buf + color_idx + 4, &color, 4);
    memcpy(color_buf + color_idx + 8, &color, 4);
    memcpy(color_buf + color_idx + 12, &color, 4);

    // update index buffer
    index_buf[index_idx + 0] = element_idx + 0;
    index_buf[index_idx + 1] = element_idx + 1;
    index_buf[index_idx + 2] = element_idx + 2;
    index_buf[index_idx + 3] = element_idx + 2;
    index_buf[index_idx + 4] = element_idx + 3;
    index_buf[index_idx + 5] = element_idx + 1;
  }

  void push_matrix() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, _width, _height, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
  }

  void pop_matrix() {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    assert(glGetError() == 0);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    assert(glGetError() == 0);
  }
};
