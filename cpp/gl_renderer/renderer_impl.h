#pragma once
#include <UIRenderFrame.h>
#include <stdint.h>

const auto BUFFER_SIZE = 16384;
class Renderer {
  int _width = 800;
  int _height = 600;
  int _triangle_count = 0;
  uint32_t _id = 0;

  float tex_buf[BUFFER_SIZE];
  float vert_buf[BUFFER_SIZE];
  uint8_t color_buf[BUFFER_SIZE];
  uint32_t index_buf[BUFFER_SIZE];

public:
  Renderer();
  ~Renderer();
  void initialize(const void *loadfunc);
  void load_atlas(int w, int h, const unsigned char *data);
  void begin(int width, int height, const UIColor32 &clr);
  void flush();
  void draw_rect(const UIRect &rect, const UIColor32 &color);
  void draw_text(const uint8_t *begin, const uint8_t *end, const UIVec2 &pos,
                 const UIColor32 &color);
  void draw_icon(int id, const UIRect &rect, const UIColor32 &color);
  void set_clip_rect(const UIRect &rect);

private:
  void push_quad(const UIRect &dst, const UIRect &src, const UIColor32 &color);
  void push_matrix();
  void pop_matrix();
};
