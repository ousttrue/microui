#include "shader.h"
#include <glad/gl.h>

class VertexCompiler {
public:
  uint32_t _id;
  VertexCompiler() { _id = glCreateShader(GL_VERTEX_SHADER); }
  ~VertexCompiler() { glDeleteShader(_id); }
  static std::shared_ptr<VertexCompiler> compile(const std::string &source) {
    auto compiler = std::make_shared<VertexCompiler>();
    auto p = source.c_str();
    glShaderSource(compiler->_id, 1, &p, nullptr);
    glCompileShader(compiler->_id);
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
    auto p = source.c_str();
    glShaderSource(compiler->_id, 1, &p, nullptr);
    glCompileShader(compiler->_id);
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
