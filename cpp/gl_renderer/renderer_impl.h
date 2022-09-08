#pragma once
#include <UIRenderFrame.h>
#include <memory>
#include <stdint.h>
#include <vector>

struct Vertex {
  float x;
  float y;
  float u;
  float v;
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

const auto BUFFER_SIZE = 16384;
class Renderer {
  int _width = 800;
  int _height = 600;

  std::shared_ptr<class VBO> vbo;
  std::shared_ptr<class IBO> ibo;
  std::shared_ptr<class Program> shader;

  std::vector<Vertex> _vertices;
  // int _triangle_count = 0;
  // float tex_buf[BUFFER_SIZE];
  // float vert_buf[BUFFER_SIZE];
  // uint8_t color_buf[BUFFER_SIZE];
  // uint32_t index_buf[BUFFER_SIZE];

public:
  std::shared_ptr<class Texture> atlas_texture;

  Renderer();
  ~Renderer();
  void initialize(const void *loadfunc);
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
