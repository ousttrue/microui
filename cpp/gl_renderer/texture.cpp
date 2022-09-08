#include "texture.h"
#include <assert.h>
#include <glad/gl.h>

//
// Texture
//
Texture::Texture() { glGenTextures(1, &_id); }
Texture::~Texture() { glDeleteTextures(1, &_id); }
void Texture::bind() { glBindTexture(GL_TEXTURE_2D, _id); }
void Texture::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

std::shared_ptr<Texture> Texture::load(int w, int h,
                                       const unsigned char *data) {
  auto texture = std::make_shared<Texture>();
  texture->bind();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE,
               data);
  texture->unbind();
  return texture;
}
