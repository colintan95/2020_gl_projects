#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>

#include <GL/glew.h>
#if defined(WIN32)
#include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "gfx_utils/window/window.h"
#include "gfx_utils/window/camera.h"
#include "gfx_utils/shader.h"
#include "gfx_utils/mesh.h"
#include "gfx_utils/texture.h"
#include "gfx_utils/primitives.h"

// TODO(colintan): Define this somewhere else
const double kPi = 3.14159265358979323846;

static const std::string vert_shader_path = "shaders/simple_light.vert";
static const std::string frag_shader_path = "shaders/simple_light.frag";

int main(int argc, char *argv[]) {

  gfx_utils::Window window;

  if (!window.Inititalize(1920, 1080, "Shadow Map")) {
    std::cerr << "Failed to initialize gfx window" << std::endl;
    exit(1);
  }

  gfx_utils::Camera camera;

  if (!camera.Initialize(&window)) {
    std::cerr << "Failed to initialize camera" << std::endl;
    exit(1);
  }

  std::vector<gfx_utils::Mesh> meshes;

  // Creates a room around the origin
  std::vector<gfx_utils::Mesh> room = gfx_utils::CreateRoom(40.f, 20.f, 80.f);
  meshes.insert(meshes.end(), std::make_move_iterator(room.begin()), 
                std::make_move_iterator(room.end()));
  
  // Enable all necessary GL settings
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  GLuint program_id;
  if (!gfx_utils::CreateProgram(&program_id, vert_shader_path, 
                                frag_shader_path)) {
    exit(1);
  }
  glUseProgram(program_id);

  GLuint vao_id;
  glGenVertexArrays(1, &vao_id);

  std::vector<GLuint> pos_vbo_id_list;
  std::vector<GLuint> normal_vbo_id_list;
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

    glm::mat4 view_mat = camera.CalcViewMatrix();
    glm::mat4 proj_mat = glm::perspective(glm::radians(30.f), 
                                          window.GetAspectRatio(),
                                          0.1f, 10000.f);

    GLint mv_mat_loc = glGetUniformLocation(program_id, "mv_mat");
    GLint mvp_mat_loc = glGetUniformLocation(program_id, "mvp_mat");
    GLint normal_mat_loc = glGetUniformLocation(program_id, "normal_mat");
    GLint mesh_color_loc = glGetUniformLocation(program_id, "mesh_color");
    GLint light_loc_loc = glGetUniformLocation(program_id, "light_loc");
    GLint camera_loc_loc = glGetUniformLocation(program_id, "camera_loc");

    glm::vec3 light_loc = glm::vec3(view_mat * glm::mat4(1.f) *
                                    glm::vec4(0.f, 0.f, 0.f, 1.f));

    glUniform3fv(light_loc_loc, 1, glm::value_ptr(light_loc));
    glUniform3fv(camera_loc_loc, 1, glm::value_ptr(camera.GetCameraLocation()));

    for (size_t i = 0; i < meshes.size(); ++i) {
      glm::mat4 model_mat = glm::mat4(1.f);
      glm::mat4 mv_mat = view_mat * model_mat;
      glm::mat4 mvp_mat = proj_mat * mv_mat;
      glUniformMatrix4fv(mv_mat_loc, 1, GL_FALSE, glm::value_ptr(mv_mat));
      glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));

      // TODO(colintan): Check that this is computing correctly
      glm::mat3 normal_mat = 
          glm::mat3(glm::transpose(glm::inverse(mv_mat)));
      glUniformMatrix3fv(normal_mat_loc, 1, GL_FALSE, 
                         glm::value_ptr(normal_mat));

      if (meshes[i].is_textured == false) {
        glUniform3fv(mesh_color_loc, 1, glm::value_ptr(meshes[i].color));
      }

      // Set vertex attributes

      glBindVertexArray(vao_id);

      glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id_list[i]);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id_list[i]);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id_list[i]);
      glDrawElements(GL_TRIANGLES, meshes[i].num_verts, GL_UNSIGNED_INT,
          (void*)0);
    }

    glBindVertexArray(0);

    window.SwapBuffers();
    window.TickMainLoop();

    if (window.ShouldQuit()) {
      should_quit = true;
    }
  }

  glDeleteBuffers(static_cast<GLsizei>(ibo_id_list.size()), &ibo_id_list[0]);
  glDeleteBuffers(static_cast<GLsizei>(normal_vbo_id_list.size()), 
      &normal_vbo_id_list[0]);
  glDeleteBuffers(static_cast<GLsizei>(pos_vbo_id_list.size()), 
      &pos_vbo_id_list[0]);

  glDeleteVertexArrays(1, &vao_id);

  glDeleteProgram(program_id);

  glfwTerminate();

  return 0;
}