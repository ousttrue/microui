#pragma once
#include <memory>
#include <stdint.h>
#include <vector>

class VBO {
  uint32_t _id = 0;
  uint32_t _count = 0;

public:
  VBO();
  ~VBO();
  static std::shared_ptr<VBO> create(uint32_t size);
  template <typename T> void update(const std::vector<T> &buffer) {
    update(buffer.data(), buffer.size() * sizeof(T), buffer.size());
  }
  void update(const void *p, uint32_t size, uint32_t count);
  void bind();
  void unbind();
  void draw();
  void clear() { _count = 0; }
};

class IBO {
  uint32_t _id = 0;
  uint32_t _count = 0;

public:
  IBO();
  ~IBO();
  static std::shared_ptr<IBO> create(uint32_t);
  template <typename T> void update(const std::vector<T> &buffer) {
    update(buffer.data(), buffer.size() * sizeof(T), buffer.size());
  }
  void update(const void *p, uint32_t size, uint32_t count);
  void bind();
  void unbind();
  void draw();
  void clear() { _count = 0; }
};

class VAO {
  uint32_t _id = 0;

public:
  std::shared_ptr<VBO> vbo;
  std::shared_ptr<IBO> ibo;

  VAO();
  ~VAO();
  static std::shared_ptr<VAO> create();

  void bind();
  void unbind();
  void draw();
};
