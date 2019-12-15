#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

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
#include "gfx_utils/program.h"
#include "gfx_utils/mesh.h"
#include "gfx_utils/material.h"
#include "gfx_utils/scene.h"
#include "gfx_utils/texture.h"
#include "gfx_utils/primitives.h"
#include "gfx_utils/lights.h"

// TODO(colintan): Define this somewhere else
const float kPi = 3.14159265358979323846f;

static const int kWindowWidth = 1920;
static const int kWindowHeight = 1080;

static const std::string vert_shader_path = "shaders/simple_light.vert";
static const std::string frag_shader_path = "shaders/simple_light.frag";

static const std::string cube_tex_directory = "assets";

static const int kShadowTexWidth = 1024;
static const int kShadowTexHeight = 1024;

int main(int argc, char* argv[]) {

  gfx_utils::Window window;

  if (!window.Inititalize(kWindowWidth, kWindowHeight, "Shadow Map")) {
    std::cerr << "Failed to initialize gfx window" << std::endl;
    exit(1);
  }

  gfx_utils::Camera camera;

  if (!camera.Initialize(&window)) {
    std::cerr << "Failed to initialize camera" << std::endl;
    exit(1);
  }

  std::vector<gfx_utils::Mesh*> meshes;
  std::vector<gfx_utils::SceneObject*> scene_objs;


  // TODO(colintan): How to get a single mesh, not having to pass in a vector
  std::vector<gfx_utils::Mesh> box_meshes;
  if (!gfx_utils::CreateMeshesFromFile(&box_meshes, "assets/", 
                                       "assets/cornell_box.obj")) {
    std::cerr << "Failed to load cornell box mesh" << std::endl;
    exit(1);
  }

  gfx_utils::SceneObject box_object;
  for (auto& mesh : box_meshes) {
    meshes.push_back(&mesh);
    box_object.AddMesh(mesh);
  }
  box_object.SetScale(glm::vec3(20.f, 20.f, 20.f));

  scene_objs.push_back(&box_object);

  // Create Lights
  std::vector<gfx_utils::SpotLight*> lights;

  gfx_utils::SpotLight white_light;
  white_light.position = glm::vec3(0.f, 20.f, 0.f);
  white_light.diffuse_intensity = glm::vec3(0.5f, 0.5f, 0.5f);
  white_light.specular_intensity = glm::vec3(0.5f, 0.5f, 0.5f);
  white_light.direction = glm::vec3(0.f, -1.f, 0.f);
  white_light.cone_angle = kPi / 1.5f;
  white_light.camera_up = glm::vec3(0.f, 0.f, -1.f);
  lights.push_back(&white_light);
          
  // Enable all necessary GL settings
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  gfx_utils::Program program;
  if (!program.CreateProgram(vert_shader_path, frag_shader_path)) {
    std::cerr << "Could not create light pass program." << std::endl;
    exit(1);
  }
  glUseProgram(program.GetProgramId());

  GLuint vao_id;
  glGenVertexArrays(1, &vao_id);

  std::vector<GLuint> pos_vbo_id_list;
  std::vector<GLuint> normal_vbo_id_list;
  std::vector<GLuint> mtl_vbo_id_list;
  std::vector<GLuint> ibo_id_list;

  // TODO(colintan): Is this a good solution?
  // Maps a mesh to the index in the buffers that contain the mesh's data
  std::unordered_map<const gfx_utils::Mesh*, int> mesh_to_idx_map;

  for (auto mesh_ptr : meshes) {
    mesh_to_idx_map[mesh_ptr] = static_cast<int>(ibo_id_list.size());

    GLuint pos_vbo_id;
    glGenBuffers(1, &pos_vbo_id); // TODO(colintan): Is bulk allocating faster?
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, mesh_ptr->pos_data.size() * 3 * sizeof(float),
                 &mesh_ptr->pos_data[0], GL_STATIC_DRAW);
    pos_vbo_id_list.push_back(pos_vbo_id);

    GLuint normal_vbo_id;
    glGenBuffers(1, &normal_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, mesh_ptr->normal_data.size() * 3 * sizeof(float),
                 &mesh_ptr->normal_data[0], GL_STATIC_DRAW);
    normal_vbo_id_list.push_back(normal_vbo_id);

    if (mesh_ptr->material_list.size() != 0) {
      GLuint mtl_vbo_id;
      glGenBuffers(1, &mtl_vbo_id);
      glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id);
      glBufferData(GL_ARRAY_BUFFER, 
                   mesh_ptr->mtl_id_data.size() * sizeof(unsigned int), 
                   &mesh_ptr->mtl_id_data[0], GL_STATIC_DRAW);
      mtl_vbo_id_list.push_back(mtl_vbo_id);
    }
    else {
      mtl_vbo_id_list.push_back(0);
    }

    GLuint ibo_id;
    glGenBuffers(1, &ibo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh_ptr->index_data.size() * sizeof(uint32_t),
                 &mesh_ptr->index_data[0], GL_STATIC_DRAW);
    ibo_id_list.push_back(ibo_id);
  }

  gfx_utils::Program shadow_program;
  if (!shadow_program.CreateProgram("shaders/shadow_pass.vert",
                                    "shaders/shadow_pass.frag")) {
    std::cerr << "Could not create shadow pass program." << std::endl;
    exit(1);
  }
  glUseProgram(shadow_program.GetProgramId());

  std::vector<GLuint> shadow_tex_id_list;
  std::vector<GLuint> shadow_fbo_id_list;

  for (int i = 0; i < lights.size(); ++i) {
    gfx_utils::SpotLight* light_ptr = lights[i];

    GLuint shadow_tex_id;
    glGenTextures(1, &shadow_tex_id);
    glBindTexture(GL_TEXTURE_2D, shadow_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kShadowTexWidth, 
                  kShadowTexHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    shadow_tex_id_list.push_back(shadow_tex_id);

    GLuint shadow_fbo_id;
    glGenFramebuffers(1, &shadow_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           shadow_tex_id, 0);
    // TODO(colintan): Possible to remove?
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shadow_fbo_id_list.push_back(shadow_fbo_id);
  }

  GLuint shadow_vao_id;
  glGenVertexArrays(1, &shadow_vao_id);

  bool should_quit = false;

  while (!should_quit) {
    // Shadow Pass
    for (int i = 0; i < lights.size(); ++i) {
      gfx_utils::SpotLight* light_ptr = lights[i];

      glUseProgram(shadow_program.GetProgramId());
      glViewport(0, 0, kShadowTexWidth, kShadowTexHeight);

      glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_id_list[i]);
      glClear(GL_DEPTH_BUFFER_BIT);

      glBindVertexArray(shadow_vao_id);

      for (size_t i = 0; i < scene_objs.size(); ++i) {
        gfx_utils::SceneObject *scene_obj_ptr = scene_objs[i];

        if (!scene_obj_ptr->HasMeshes()) {
          continue;
        }

        for (auto mesh_ptr : scene_obj_ptr->GetMeshes()) {
          glm::mat4 model_mat = scene_obj_ptr->CalcTransform();
          glm::mat4 view_mat = 
            glm::lookAt(light_ptr->position, 
                        light_ptr->position + light_ptr->direction, 
                        light_ptr->camera_up);
          glm::mat4 proj_mat = glm::perspective(light_ptr->cone_angle, 
                                                1.f, 5.f, 30.f);
          glm::mat4 mvp_mat = proj_mat * view_mat * model_mat; 

          shadow_program.GetUniform("mvp_mat").Set(mvp_mat);

          int mesh_idx = mesh_to_idx_map[mesh_ptr];

          glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id_list[mesh_idx]);
          glEnableVertexAttribArray(0);
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id_list[mesh_idx]);
          glDrawElements(GL_TRIANGLES, mesh_ptr->num_verts, GL_UNSIGNED_INT,
              (void*)0);
        }    
      }

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);                                       

    // Light Pass
    glUseProgram(program.GetProgramId());
    glViewport(0, 0, kWindowWidth, kWindowHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view_mat = camera.CalcViewMatrix();
    glm::mat4 proj_mat = glm::perspective(glm::radians(30.f), 
                                          window.GetAspectRatio(),
                                          0.1f, 1000.f);

    program.GetUniform("camera_pos").Set(camera.GetCameraLocation());

    for (int i = 0; i < lights.size(); ++i) {
      gfx_utils::SpotLight* light_ptr = lights[i];

      // Position of light in modelview space
      glm::vec3 pos_mv = 
          glm::vec3(view_mat * glm::vec4(light_ptr->position, 1.f));
      glm::vec3 dir_mv = 
          glm::vec3(glm::mat3(glm::transpose(glm::inverse(view_mat))) * 
                    light_ptr->direction);

      program.GetUniform("lights", i, "position_mv").Set(pos_mv);
      program.GetUniform("lights", i, "diffuse_intensity")
             .Set(light_ptr->diffuse_intensity);
      program.GetUniform("lights", i, "specular_intensity")
             .Set(light_ptr->specular_intensity);
      program.GetUniform("lights", i, "direction_mv").Set(dir_mv);
      program.GetUniform("lights", i, "cone_angle")
             .Set(light_ptr->cone_angle);
    }                  

    program.GetUniform("ambient_intensity").Set(glm::vec3(0.5f, 0.5f, 0.5f));

    for (size_t i = 0; i < scene_objs.size(); ++i) {
      gfx_utils::SceneObject *scene_obj_ptr = scene_objs[i];

      if (!scene_obj_ptr->HasMeshes()) {
        continue;
      }

      for (auto mesh_ptr : scene_obj_ptr->GetMeshes()) {
        glm::mat4 model_mat = scene_obj_ptr->CalcTransform();
        glm::mat4 mv_mat = view_mat * model_mat;
        glm::mat4 mvp_mat = proj_mat * mv_mat;

        program.GetUniform("mv_mat").Set(mv_mat);
        program.GetUniform("mvp_mat").Set(mvp_mat);

        // TODO(colintan): Check that this is computing correctly
        glm::mat3 normal_mat = 
            glm::mat3(glm::transpose(glm::inverse(mv_mat)));

        program.GetUniform("normal_mat").Set(normal_mat);

        // Set the materials data in the fragment shader

        // TODO(colintan): Modify this so that we can render a mesh that has
        // multiple materials
        const auto& mtl_list = mesh_ptr->material_list;
        for (int i = 0; i < mtl_list.size(); ++i) {
          const auto& mtl = mtl_list[i];

          program.GetUniform("materials", i, "ambient_color")
                 .Set(mtl.ambient_color);
          program.GetUniform("materials", i, "diffuse_color")
                 .Set(mtl.diffuse_color);
          program.GetUniform("materials", i, "specular_color")
                 .Set(mtl.specular_color);
          program.GetUniform("materials", i, "emission_color")
                 .Set(mtl.emission_color);
          program.GetUniform("materials", i, "shininess")
                 .Set(mtl.shininess);                                               
        }

        for (int i = 0; i < lights.size(); ++i) {
          gfx_utils::SpotLight* light_ptr = lights[i];

          program.GetUniform("lights", i, "is_active").Set(1);

          // Position of light in modelview space
          glm::vec3 pos_mv = 
              glm::vec3(view_mat * glm::vec4(light_ptr->position, 1.f));
          glm::vec3 dir_mv = 
              glm::vec3(glm::mat3(glm::transpose(glm::inverse(view_mat))) * 
                        light_ptr->direction);

          program.GetUniform("lights", i, "position_mv").Set(pos_mv);
          program.GetUniform("lights", i, "diffuse_intensity")
                .Set(light_ptr->diffuse_intensity);
          program.GetUniform("lights", i, "specular_intensity")
                .Set(light_ptr->specular_intensity);
          program.GetUniform("lights", i, "direction_mv").Set(dir_mv);
          program.GetUniform("lights", i, "cone_angle")
                .Set(light_ptr->cone_angle);

          // // TODO(colintan): Don't hardcode this
          glActiveTexture(GL_TEXTURE10 + i);    
          glBindTexture(GL_TEXTURE_2D, shadow_tex_id_list[i]);

          program.GetUniform("lights", i, "shadow_tex").Set(10 + i); 

          glm::mat4 light_view_mat = 
            glm::lookAt(light_ptr->position, 
                        light_ptr->position + light_ptr->direction, 
                        light_ptr->camera_up);
          glm::mat4 light_proj_mat = glm::perspective(light_ptr->cone_angle, 
                                                      1.f, 5.f, 30.f);

          glm::mat4 shadow_mat = light_proj_mat * light_view_mat * 
              model_mat;

          program.GetUniform("shadow_mats", i).Set(shadow_mat);      
        }    

        // Set vertex attributes

        glBindVertexArray(vao_id);

        int mesh_idx = mesh_to_idx_map[mesh_ptr];

        glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id_list[mesh_idx]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id_list[mesh_idx]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        if (mesh_ptr->material_list.size() != 0) {
          glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id_list[mesh_idx]);
          glEnableVertexAttribArray(3);
          glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, 0, (GLvoid*)0);
        }
        else {
          // TODO(colintan): Disable materials for this mesh
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id_list[mesh_idx]);
        glDrawElements(GL_TRIANGLES, mesh_ptr->num_verts, GL_UNSIGNED_INT,
            (void*)0);
      }
    }

    glBindVertexArray(0);

    window.SwapBuffers();
    window.TickMainLoop();

    if (window.ShouldQuit()) {
      should_quit = true;
    }
  }

  glDeleteVertexArrays(1, &shadow_vao_id);
  if (!shadow_fbo_id_list.empty()) {
    glDeleteFramebuffers(static_cast<GLsizei>(shadow_fbo_id_list.size()), 
                        &shadow_fbo_id_list[0]);
  }
  if (!shadow_tex_id_list.empty()) {
    glDeleteTextures(static_cast<GLsizei>(shadow_tex_id_list.size()), 
                    &shadow_tex_id_list[0]);
  }
  shadow_program.DestroyProgram();

  glDeleteBuffers(static_cast<GLsizei>(ibo_id_list.size()), &ibo_id_list[0]);
  glDeleteBuffers(static_cast<GLsizei>(mtl_vbo_id_list.size()), 
                  &mtl_vbo_id_list[0]);
  glDeleteBuffers(static_cast<GLsizei>(normal_vbo_id_list.size()), 
      &normal_vbo_id_list[0]);
  glDeleteBuffers(static_cast<GLsizei>(pos_vbo_id_list.size()), 
      &pos_vbo_id_list[0]);

  glDeleteVertexArrays(1, &vao_id);

  program.DestroyProgram();

  glfwTerminate();

  return 0;
}