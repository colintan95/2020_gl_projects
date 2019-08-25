#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include "GL/glew.h"
#if defined(WIN32)
#include "GL/gl.h"
#endif
#include "GLFW/glfw3.h"

#include "graphics_utils/shader.h"

static const std::string vert_shader_path = "shaders/screen_color.vert";
static const std::string frag_shader_path = "shaders/screen_color.frag";

// Pos: .., .., .., Color: .., .., ..
static const GLfloat vert_data[] = {
  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
   0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
   0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
};

// The dimensions of the vectors for each type of vertex data
static const int pos_dim = 3;
static const int color_dim = 3;

static const int vert_count = 3;

static constexpr int vert_stride = 
    (pos_dim + color_dim) * sizeof(GLfloat); // in bytes

static constexpr int pos_offset   = 0 * sizeof(GLfloat); // in bytes
static constexpr int color_offset = pos_dim * sizeof(GLfloat); // in bytes

// TODO(colintan): How to destroy OpenGL resources when we exit partway
// because of an error

int main(int argc, char *argv[]) {

  // TODO(colintan): Check if we need glewExperimental to be true
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    exit(1);
  }

  std::cout << "GLFW initialized" << std::endl;

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // TODO(colintan): Check if we really need this
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *window;
  window = glfwCreateWindow(1024, 768, "Hello Triangle", nullptr, nullptr);

  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    exit(1);
  }

  glfwMakeContextCurrent(window);

  // TODO(colintan): Is this needed?
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    exit(1);
  }

  std::cout << "GLEW initialized" << std::endl;

  std::string vert_shader_src;
  if (!LoadShaderSource(&vert_shader_src, vert_shader_path)) {
    std::cerr << "Could not find vertex shader source at " 
        << vert_shader_path  << std::endl;
    exit(1);
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

    exit(1);
  }
  
  std::string frag_shader_src;
  if (!LoadShaderSource(&frag_shader_src, frag_shader_path)) {
    std::cerr << "Could not find fragment shader source at: "
      << frag_shader_path << std::endl;
    exit(1);
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

    exit(1);
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

    exit(1);
  }

  glDeleteShader(frag_shader_id);
  glDeleteShader(vert_shader_id);

  glUseProgram(program_id);

  GLuint vao_id;
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GLuint vbo_id;
  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vert_data), vert_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0,                  // index
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized
    vert_stride,        // stride
    (GLvoid*)pos_offset // pointer
  );

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1,                    // index
    3,                    // size
    GL_FLOAT,             // type
    GL_FALSE,             // normalized
    vert_stride,          // stride
    (GLvoid*)color_offset // pointer
  );

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  bool should_quit = false;

  while (!should_quit) {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao_id);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();

    if (glfwWindowShouldClose(window) == 1) {
      should_quit = true;
    }
  }

  glDeleteBuffers(1, &vbo_id);
  glDeleteVertexArrays(1, &vao_id);

  glDeleteProgram(program_id);

  glfwTerminate();

  return 0;
}