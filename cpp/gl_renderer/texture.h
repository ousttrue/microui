#pragma once
#include <memory>
#include <stdint.h>

class Texture {
  uint32_t _id = 0;

public:
  Texture();
  ~Texture();
  static std::shared_ptr<Texture> load(int w, int h, const unsigned char *data);
  void bind();
  void unbind();
};
