#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include <GL/glew.h>
#if defined(WIN32)
#include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gfx_utils/shader.h"
#include "gfx_utils/mesh.h"
#include "gfx_utils/texture.h"

// TODO(colintan): Define this somewhere else
const double kPi = 3.14159265358979323846;

static const std::string vert_shader_path = "shaders/mesh.vert";
static const std::string frag_shader_path = "shaders/mesh.frag";

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

  // TODO(colintan): Is glewExperimental needed?
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    exit(1);
  }

  std::cout << "GLEW initialized" << std::endl;

  std::vector<gfx_utils::Mesh> meshes;
  if (!gfx_utils::CreateMeshesFromFile(&meshes, "assets/models/sponza.obj")) {
    std::cerr << "Failed to load mesh" << std::endl;
    exit(1);
  }

  std::cout << "Number of meshes loaded: " << meshes.size() << std::endl;

/*
  gfx_utils::Mesh mesh;
  if (!gfx_utils::CreateMeshFromFile(&mesh, "assets/teapot.obj")) {
    std::cerr << "Failed to load mesh" << std::endl;
    exit(1);
  }

  gfx_utils::Texture texture;
  if (!gfx_utils::CreateTextureFromFile(&texture, "assets/teapot_tex.jpg")) {
    std::cerr << "Failed to load texture" << std::endl;
    exit(1);
  }
*/
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
  glm::mat4 model_mat = glm::mat4(1.f);
  glm::mat4 view_mat = 
    glm::translate(glm::mat4(1.f), glm::vec3(0.f, -20.f, 0.f)) *
    glm::rotate(glm::mat4(1.f), (static_cast<float>(kPi) / 2.f),
        glm::vec3(0.f, 1.f, 0.f));
  //glm::mat4 view_mat =
  //  glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -60.f)) *
  //  glm::rotate(glm::mat4(1.f), (static_cast<float>(kPi) / 8.f),
  //    glm::vec3(1.f, 0.f, 0.f));
  glm::mat4 proj_mat = glm::perspective(glm::radians(30.f), 4.f / 3.f,
                                        0.1f, 10000.f);
  glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;

  GLint mvp_mat_loc = glGetUniformLocation(program_id, "mvp_mat");
  glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));
/*
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
*/
  GLuint vao_id;
  glGenVertexArrays(1, &vao_id);

  std::vector<GLuint> pos_vbo_id_list;
  std::vector<GLuint> normal_vbo_id_list;
  std::vector<GLuint> texcoord_vbo_id_list;
  std::vector<GLuint> mtl_vbo_id_list;
  std::vector<GLuint> ibo_id_list;

  for (const auto& mesh : meshes) {
    GLuint pos_vbo_id;
    glGenBuffers(1, &pos_vbo_id); // TODO(colintan): Is bulk allocating faster?
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, mesh.pos_data.size() * 3 * sizeof(float),
                 &mesh.pos_data[0], GL_STATIC_DRAW);
    pos_vbo_id_list.push_back(pos_vbo_id);

    GLuint normal_vbo_id;
    glGenBuffers(1, &normal_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, mesh.normal_data.size() * 3 * sizeof(float),
      &mesh.normal_data[0], GL_STATIC_DRAW);
    normal_vbo_id_list.push_back(normal_vbo_id);

    GLuint texcoord_vbo_id;
    glGenBuffers(1, &texcoord_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, mesh.texcoord_data.size() * 2 * sizeof(float),
      &mesh.texcoord_data[0], GL_STATIC_DRAW);
    texcoord_vbo_id_list.push_back(texcoord_vbo_id);

    GLuint mtl_vbo_id;
    glGenBuffers(1, &mtl_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, 
      mesh.mtl_id_data.size() * sizeof(unsigned int), 
      &mesh.mtl_id_data[0], 
      GL_STATIC_DRAW);
    mtl_vbo_id_list.push_back(mtl_vbo_id);

    GLuint ibo_id;
    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
      mesh.index_data.size() * sizeof(uint32_t),
      &mesh.index_data[0],
      GL_STATIC_DRAW);
    ibo_id_list.push_back(ibo_id);
  }

  bool should_quit = false;

  while (!should_quit) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < meshes.size(); ++i) {
      glBindVertexArray(vao_id);

      glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id_list[i]);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id_list[i]);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id_list[i]);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id_list[i]);
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, 0, (GLvoid*)0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id_list[i]);
      glDrawElements(GL_TRIANGLES, meshes[i].num_verts, GL_UNSIGNED_INT,
          (void*)0);
    }

    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();

    if (glfwWindowShouldClose(window) == 1) {
      should_quit = true;
    }
  }

  glDeleteBuffers(ibo_id_list.size(), &ibo_id_list[0]);

  glDeleteBuffers(mtl_vbo_id_list.size(), &mtl_vbo_id_list[0]);
  glDeleteBuffers(texcoord_vbo_id_list.size(), &texcoord_vbo_id_list[0]);
  glDeleteBuffers(normal_vbo_id_list.size(), &normal_vbo_id_list[0]);
  glDeleteBuffers(pos_vbo_id_list.size(), &pos_vbo_id_list[0]);

  glDeleteVertexArrays(1, &vao_id);

  glDeleteProgram(program_id);

  glfwTerminate();

  return 0;
}