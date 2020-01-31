#include "gfx_utils/program.h"

#include <cassert>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include <fstream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

namespace gfx_utils {

void Uniform::Set(bool val) {
  glUniform1i(location_, val ? 1 : 0);
}

void Uniform::Set(int val) {
  glUniform1i(location_, val);
}

void Uniform::Set(float val) {
  glUniform1f(location_, val);
}

void Uniform::Set(const glm::vec3& val) {
  glUniform3fv(location_, 1, glm::value_ptr(val));
}

void Uniform::Set(const glm::mat3& val) {
  glUniformMatrix3fv(location_, 1, GL_FALSE, glm::value_ptr(val));
}

void Uniform::Set(const glm::mat4& val) {
  glUniformMatrix4fv(location_, 1, GL_FALSE, glm::value_ptr(val));
}

bool Program::CreateFromFiles(const std::string& vert_shader_path, 
                              const std::string& frag_shader_path,
                              const std::string& geom_shader_path) {
  std::string vert_shader_src;
  if (!LoadShaderSource(&vert_shader_src, vert_shader_path)) {
    std::cerr << "Failed to find vertex shader source at "
              << vert_shader_path << std::endl;
    return false;
  }            

  std::string frag_shader_src;
  if (!LoadShaderSource(&frag_shader_src, frag_shader_path)) {
    std::cerr << "Failed to find fragment shader source at: "
              << frag_shader_path << std::endl;
    return false;
  }

  std::string geom_shader_src = "";
  if (!geom_shader_path.empty()) {
    if (!LoadShaderSource(&geom_shader_src, geom_shader_path)) {
      std::cerr << "Failed to find geometry shader source at: "
                << geom_shader_path << std::endl;
      return false;
    }
  }

  return CreateFromSource(vert_shader_src, frag_shader_src, geom_shader_src);
}

bool Program::CreateFromSource(const std::string& vert_shader_src, 
                               const std::string& frag_shader_src,
                               const std::string& geom_shader_src) {

  GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
  if (!CompilerShader(vert_shader_id, vert_shader_src.c_str())) {
    return false;
  }
  
  GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  if (!CompilerShader(frag_shader_id, frag_shader_src.c_str())) {
    return false;
  }

  GLuint geom_shader_id = 0;
  if (!geom_shader_src.empty()) {
    geom_shader_id = glCreateShader(GL_GEOMETRY_SHADER);
    if (!CompilerShader(geom_shader_id, geom_shader_src.c_str())) {
      return false;
    }
  }

  GLuint program_id = glCreateProgram();

  glAttachShader(program_id, vert_shader_id);
  glAttachShader(program_id, frag_shader_id);

  if (geom_shader_id != 0) {
    glAttachShader(program_id, geom_shader_id);
  }

  glLinkProgram(program_id);

  // Check that the program was successfully created
  GLint gl_result = GL_FALSE;
  glGetProgramiv(program_id, GL_LINK_STATUS, &gl_result);
  if (gl_result == GL_FALSE) {
    int log_length;
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 0) {
      std::vector<GLchar> error_log(log_length);
      glGetProgramInfoLog(program_id, log_length, nullptr, &error_log[0]);
      std::cerr << &error_log[0] << std::endl;
    }

    return false;
  }

  if (geom_shader_id != 0) {
    glDeleteShader(geom_shader_id);
  }

  glDeleteShader(frag_shader_id);
  glDeleteShader(vert_shader_id);   

  program_id_ = program_id;

  is_created_ = true;

  return true;
}

void Program::Destroy() {
  assert(is_created_);

  glDeleteProgram(program_id_);
}

Uniform Program::GetUniform(const std::string& var) {
  return Uniform(glGetUniformLocation(program_id_, var.c_str()));
}

Uniform Program::GetUniform(const std::string& array, int index, 
                            const std::string& var) {
  std::string actual_var = array + "[" + std::to_string(index) + "]." + var;

  return Uniform(glGetUniformLocation(program_id_, actual_var.c_str()));                         
}

Uniform Program::GetUniform(const std::string& array, int index) {
  std::string var = array + "[" + std::to_string(index) + "]";

  return Uniform(glGetUniformLocation(program_id_, var.c_str()));                         
}

bool Program::LoadShaderSource(std::string* out_str, const std::string& path) {
  std::ifstream shader_stream(path, std::ios::in);

  if (!shader_stream.is_open()) {
    *out_str = "";
    return false;
  }

  std::stringstream str_stream;
  str_stream << shader_stream.rdbuf();
  *out_str = str_stream.str();

  shader_stream.close();

  return true;
}

bool Program::CompilerShader(GLuint shader_id, const char* shader_src) {
  glShaderSource(shader_id, 1, &shader_src, nullptr);
  glCompileShader(shader_id);

  // Check success of the fragment shader compilation 
  GLint gl_result = GL_FALSE;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &gl_result);

  if (gl_result == GL_FALSE) {
    int log_length;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 0) {
      std::vector<GLchar> error_log(log_length);
      glGetShaderInfoLog(shader_id, log_length, nullptr, &error_log[0]);
      std::cerr << &error_log[0] << std::endl;
    }

    return false;
  }

  return true;
}

} // namespace gfx_utils