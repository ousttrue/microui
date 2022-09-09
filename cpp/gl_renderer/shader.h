#pragma once
#include <memory>
#include <stdint.h>
#include <string>

class Program {
  uint32_t _id;

public:
  Program();
  ~Program();
  static std::shared_ptr<Program> create(const std::string &vs,
                                         const std::string &fs);
  void bind();
  void unbind();
  void set_uniform_matrix(const std::string &key, const float *m,
                          bool transpose);
};
