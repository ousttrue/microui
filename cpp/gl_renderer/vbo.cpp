#include "vbo.h"
#include <glad/gl.h>

VBO::VBO() { glGenBuffers(1, &_id); }
VBO::~VBO() { glDeleteBuffers(1, &_id); }
std::shared_ptr<VBO> VBO::create(uint32_t size) {
  auto vbo = std::make_shared<VBO>();
  vbo->bind();
  glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
  vbo->unbind();
  return vbo;
}
void VBO::bind() { glBindBuffer(GL_ARRAY_BUFFER, _id); }
void VBO::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
void VBO::update(const void *p, uint32_t size, uint32_t count) {
  bind();
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, p);
  _count = count;
  unbind();
}
void VBO::draw() { glDrawArrays(GL_TRIANGLES, 0, _count); }

IBO::IBO() { glGenBuffers(1, &_id); }
IBO::~IBO() { glDeleteBuffers(1, &_id); }
std::shared_ptr<IBO> IBO::create(uint32_t size) {
  auto ibo = std::make_shared<IBO>();
  ibo->bind();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
  ibo->unbind();
  return ibo;
}
void IBO::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id); }
void IBO::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
void IBO::update(const void *p, uint32_t size, uint32_t count) {
  bind();
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, p);
  _count = count;
  unbind();
}
void IBO::draw() { glDrawElements(GL_TRIANGLES, _count, GL_UNSIGNED_INT, 0); }

VAO::VAO() { glGenVertexArrays(1, &_id); }
VAO::~VAO() { glDeleteVertexArrays(1, &_id); }
std::shared_ptr<VAO> VAO::create() {
  auto vao = std::make_shared<VAO>();
  return vao;
}
void VAO::bind() { glBindVertexArray(_id); }
void VAO::unbind() { glBindVertexArray(0); }
void VAO::draw() {
  bind();
  if (ibo) {
    ibo->draw();
  } else if (vbo) {
    vbo->draw();
  }
  unbind();
}
