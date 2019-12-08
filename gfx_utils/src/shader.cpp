#include "gfx_utils/shader.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include <fstream>
#include <sstream>

namespace gfx_utils {

bool CreateProgram(GLuint *out_program_id,
                   const std::string& vert_shader_path,
                   const std::string& frag_shader_path) {

  *out_program_id = 0;

  std::string vert_shader_src;
  if (!gfx_utils::LoadShaderSource(&vert_shader_src, vert_shader_path)) {
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
  if (!gfx_utils::LoadShaderSource(&frag_shader_src, frag_shader_path)) {
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

  *out_program_id = program_id;

  return true;
}

bool LoadShaderSource(std::string* out_str, const std::string& path) {
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

}