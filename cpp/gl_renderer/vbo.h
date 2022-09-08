#pragma once
#include <memory>
#include <stdint.h>

class VBO {
  uint32_t _id = 0;
  uint32_t _size = 0;
  uint32_t _count = 0;

public:
  VBO();
  ~VBO();
  static std::shared_ptr<VBO> create(uint32_t size);
  void update(const void *p, uint32_t size, uint32_t count);
  void bind();
  void unbind();
  void draw();
  void clear() { _count = 0; }
};

class IBO {
  uint32_t _id = 0;
  uint32_t _size = 0;
  uint32_t _count = 0;

public:
  IBO();
  ~IBO();
  static std::shared_ptr<IBO> create(uint32_t);
  void update(const void *p, uint32_t size);
  void bind();
  void unbind();
  void draw();
  void clear() { _count = 0; }
};
