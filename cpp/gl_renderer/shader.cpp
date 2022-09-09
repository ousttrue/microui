#include "shader.h"
#include <glad/gl.h>
#include <iostream>>

inline bool compile_shader(uint32_t id, const char *src, char *log,
                           GLsizei log_len) {
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);
  GLint vertex_compiled;
  glGetShaderiv(id, GL_COMPILE_STATUS, &vertex_compiled);
  if (vertex_compiled == GL_TRUE) {
    return true;
  }
  GLsizei log_length = 0;
  glGetShaderInfoLog(id, log_len, &log_length, log);
  return false;
}

class VertexCompiler {
public:
  uint32_t _id;
  VertexCompiler() { _id = glCreateShader(GL_VERTEX_SHADER); }
  ~VertexCompiler() { glDeleteShader(_id); }
  static std::shared_ptr<VertexCompiler> compile(const std::string &source) {
    auto compiler = std::make_shared<VertexCompiler>();
    char buf[1024];
    if (!compile_shader(compiler->_id, source.c_str(), buf, sizeof(buf))) {
      std::cerr << buf << std::endl;
      return nullptr;
    }
    return compiler;
  }
};

class FragmentCompiler {
public:
  uint32_t _id;
  FragmentCompiler() { _id = glCreateShader(GL_FRAGMENT_SHADER); }
  ~FragmentCompiler() { glDeleteShader(_id); }
  static std::shared_ptr<FragmentCompiler> compile(const std::string &source) {
    auto compiler = std::make_shared<FragmentCompiler>();
    char buf[1024];
    if (!compile_shader(compiler->_id, source.c_str(), buf, sizeof(buf))) {
      std::cerr << buf << std::endl;
      return nullptr;
    }
    return compiler;
  }
};

Program::Program() { _id = glCreateProgram(); }
Program::~Program() { glDeleteProgram(_id); }
std::shared_ptr<Program> Program::create(const std::string &vs_source,
                                         const std::string &fs_source) {

  auto vs = VertexCompiler::compile(vs_source);
  if (!vs) {
    return nullptr;
  }
  auto fs = FragmentCompiler::compile(fs_source);
  if (!fs) {
    return nullptr;
  }

  // link
  auto program = std::make_shared<Program>();
  glAttachShader(program->_id, vs->_id);
  glAttachShader(program->_id, fs->_id);
  glLinkProgram(program->_id);
  return program;
}
void Program::bind() { glUseProgram(_id); }
void Program::unbind() { glUseProgram(0); }
void Program::set_uniform_matrix(const std::string &key, const float *m,
                                 bool transpose) {
  auto location = glGetUniformLocation(_id, key.c_str());
  glUniformMatrix4fv(location, 1, transpose, m);
}
