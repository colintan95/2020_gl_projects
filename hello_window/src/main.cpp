#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include <GL/glew.h>
#if defined(WIN32)
#include <GL/gl.h>
#endif

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gfx_utils/window/window.h"
#include "gfx_utils/window/camera.h"
#include "gfx_utils/shader.h"
#include "gfx_utils/mesh.h"
#include "gfx_utils/texture.h"

// TODO(colintan): Define this somewhere else
const double kPi = 3.14159265358979323846;

static const std::string vert_shader_path = "shaders/textured_mesh.vert";
static const std::string frag_shader_path = "shaders/textured_mesh.frag";

int main(int argc, char *argv[]) {

  gfx_utils::Window window;

  if (!window.Inititalize(1024, 768, "Hello Window")) {
    std::cerr << "Failed to initialize gfx window" << std::endl;
    exit(1);
  }

  gfx_utils::Camera camera;

  if (!camera.Initialize(&window)) {
    std::cerr << "Failed to initialize camera" << std::endl;
    exit(1);
  }
  
  std::vector<gfx_utils::Mesh> meshes;
  if (!gfx_utils::CreateMeshesFromFile(&meshes, "assets/teapot.obj")) {
    std::cerr << "Failed to load mesh" << std::endl;
    exit(1);
  }

  gfx_utils::Mesh mesh = meshes[0];

  gfx_utils::Texture texture;
  if (!gfx_utils::CreateTextureFromFile(&texture, "assets/teapot_tex.jpg")) {
    std::cerr << "Failed to load texture" << std::endl;
    exit(1);
  }

  // Enable all necessary GL settings
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  std::string vert_shader_src;
  if (!gfx_utils::LoadShaderSource(&vert_shader_src, vert_shader_path)) {
    std::cerr << "Failed to find vertex shader source at "
      << vert_shader_path << std::endl;
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
  if (!gfx_utils::LoadShaderSource(&frag_shader_src, frag_shader_path)) {
    std::cerr << "Failed to find fragment shader source at: "
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

  GLuint program_id;  
  if (!gfx_utils::CreateProgram(&program_id, vert_shader_path, 
                                frag_shader_path)) {
    exit(1);
  }
  glUseProgram(program_id);

  GLuint texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.tex_width, texture.tex_height, 
               0, GL_RGB, GL_UNSIGNED_BYTE, &texture.tex_data[0]);
  glActiveTexture(GL_TEXTURE0);
  GLint texture_loc = glGetUniformLocation(program_id, "mesh_texture");
  glUniform1i(texture_loc, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  GLuint vao_id;
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GLuint pos_vbo_id;
  glGenBuffers(1, &pos_vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
  glBufferData(GL_ARRAY_BUFFER, mesh.pos_data.size() * 3 * sizeof(float), 
               &mesh.pos_data[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  GLuint texcoord_vbo_id;
  glGenBuffers(1, &texcoord_vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id);
  glBufferData(GL_ARRAY_BUFFER, mesh.texcoord_data.size() * 2 * sizeof(float), 
               &mesh.texcoord_data[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  GLuint ibo_id;
  glGenBuffers(1, &ibo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
               mesh.index_data.size() * sizeof(uint32_t),
               &mesh.index_data[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  bool should_quit = false;

  while (!should_quit) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model_mat = 
      glm::translate(glm::mat4(1.f), glm::vec3(0.f, -7.5f, 0.f)) *
      glm::rotate(glm::mat4(1.f), -(static_cast<float>(kPi)/2.f),
                  glm::vec3(1.f, 0.f, 0.f));
    glm::mat4 view_mat = camera.CalcViewMatrix();
    glm::mat4 proj_mat = glm::perspective(glm::radians(30.f),
                                          window.GetAspectRatio(),
                                          0.1f, 100.f);
    glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;

    GLint mvp_mat_loc = glGetUniformLocation(program_id, "mvp_mat");
    glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glBindVertexArray(vao_id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glDrawElements(GL_TRIANGLES,
                   mesh.num_verts,
                   GL_UNSIGNED_INT,
                   (void*)0);
    
    glBindVertexArray(0);

    window.SwapBuffers();
    window.TickMainLoop();

    if (window.ShouldQuit()) {
      should_quit = true;
    }
  }

  glDeleteBuffers(1, &ibo_id);

  glDeleteBuffers(1, &texcoord_vbo_id);
  glDeleteBuffers(1, &pos_vbo_id);
  glDeleteVertexArrays(1, &vao_id);

  glDeleteTextures(1, &texture_id);
  
  glDeleteProgram(program_id);

  glfwTerminate();

  return 0;
}