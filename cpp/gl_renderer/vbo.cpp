#include "vbo.h"
#include <glad/gl.h>

VBO::VBO() { glGenBuffers(1, &_id); }
VBO::~VBO() { glDeleteBuffers(1, &_id); }
std::shared_ptr<VBO> VBO::create(uint32_t size) {
  auto vbo = std::make_shared<VBO>();
  vbo->bind();
  glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
  vbo->_size = size;
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

IBO::IBO() { glGenBuffers(1, &_id); }
IBO::~IBO() { glDeleteBuffers(1, &_id); }
std::shared_ptr<IBO> IBO::create(uint32_t size) {
  auto ibo = std::make_shared<IBO>();
  ibo->bind();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
  ibo->_size = size;
  ibo->unbind();
  return ibo;
}
void IBO::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id); }
void IBO::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
void IBO::update(const void *p, uint32_t size) {
  bind();
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, p);
  _count = size / 4;
  unbind();
}
void IBO::draw() {
  bind();
  //   glTexCoordPointer(2, GL_FLOAT, 0, tex_buf);
  //   glVertexPointer(2, GL_FLOAT, 0, vert_buf);
  //   glColorPointer(4, GL_UNSIGNED_BYTE, 0, color_buf);
  glDrawElements(GL_TRIANGLES, _count, GL_UNSIGNED_INT, 0);
  unbind();
}
