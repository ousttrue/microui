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
  // uint8_t r;
  // uint8_t g;
  // uint8_t b;
  // uint8_t a;
  UIColor32 color;
};

class Renderer {
  uint32_t _width = 1;
  uint32_t _height = 1;
  float _matrix[16] = {
      1, 0, 0, 0, //
      0, 1, 0, 0, //
      0, 0, 1, 0, //
      0, 0, 0, 1, //
  };

  std::shared_ptr<class VAO> vao;
  std::shared_ptr<class Program> shader;

  std::vector<Vertex> _vertices;
  std::vector<uint32_t> _indices;

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
  void push_quad(const UIRect &quad, const UIRect &glyph,
                 const UIColor32 &color);
};
