#include "gl_utils/program.h"

#include <cassert>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include <fstream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

namespace gl_utils {

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

bool Program::CreateProgram(const std::string& vert_shader_path, 
                            const std::string& frag_shader_path) {
  std::string vert_shader_src;
  if (!LoadShaderSource(&vert_shader_src, vert_shader_path)) {
    std::cerr << "Failed to find vertex shader source at "
              << vert_shader_path << std::endl;
    return false;
  }            

  GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);

  char const *vert_shader_src_ptr = vert_shader_src.c_str();
  glShaderSource(vert_shader_id, 1, &vert_shader_src_ptr, nullptr);
  glCompileShader(vert_shader_id);

  GLint gl_result;

  // Check success of the vertex shader compilation  
  gl_result = GL_FALSE;
  glGetShaderiv(vert_shader_id, GL_COMPILE_STATUS, &gl_result);
  if (gl_result == GL_FALSE) {
    int log_length;
    glGetShaderiv(vert_shader_id, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 0) {
      std::vector<GLchar> error_log(log_length);
      glGetShaderInfoLog(vert_shader_id, log_length, nullptr, &error_log[0]);
      std::cerr << &error_log[0] << std::endl;
    }

    return false;
  }
  
  std::string frag_shader_src;
  if (!LoadShaderSource(&frag_shader_src, frag_shader_path)) {
    std::cerr << "Failed to find fragment shader source at: "
      << frag_shader_path << std::endl;
    return false;
  }

  GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

  char const* frag_shader_src_ptr = frag_shader_src.c_str();
  glShaderSource(frag_shader_id, 1, &frag_shader_src_ptr, nullptr);
  glCompileShader(frag_shader_id);

  // Check success of the fragment shader compilation 
  gl_result = GL_FALSE;
  glGetShaderiv(frag_shader_id, GL_COMPILE_STATUS, &gl_result);
  if (gl_result == GL_FALSE) {
    int log_length;
    glGetShaderiv(frag_shader_id, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length > 0) {
      std::vector<GLchar> error_log(log_length);
      glGetShaderInfoLog(frag_shader_id, log_length, nullptr, &error_log[0]);
      std::cerr << &error_log[0] << std::endl;
    }

    return false;
  }

  GLuint program_id = glCreateProgram();
  glAttachShader(program_id, vert_shader_id);
  glAttachShader(program_id, frag_shader_id);
  glLinkProgram(program_id);

  // Check that the program was successfully created
  gl_result = GL_FALSE;
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

  glDeleteShader(frag_shader_id);
  glDeleteShader(vert_shader_id);   

  program_id_ = program_id;

  is_created_ = true;

  return true;
}

void Program::DestroyProgram() {
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

} // namespace gl_utils