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
#include "gfx_utils/program.h"
#include "gfx_utils/mesh.h"
#include "gfx_utils/material.h"
#include "gfx_utils/scene.h"
#include "gfx_utils/texture.h"
#include "gfx_utils/primitives.h"

// TODO(colintan): Define this somewhere else
const float kPi = 3.14159265358979323846f;

static const std::string vert_shader_path = "shaders/simple_light.vert";
static const std::string frag_shader_path = "shaders/simple_light.frag";

static const std::string cube_tex_directory = "assets/cube";

int main(int argc, char* argv[]) {

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

  std::vector<gfx_utils::Mesh*> meshes;

  // Create wall mesh
  gfx_utils::Mesh wall = 
      gfx_utils::CreatePlane(glm::vec3(-10.f,  10.f, 0.f),
                             glm::vec3(-10.f, -10.f, 0.f),
                             glm::vec3( 10.f, -10.f, 0.f),
                             glm::vec3( 10.f,  10.f, 0.f));
  wall.mtl_id_data = {0, 0, 0, 0, 0, 0};
  gfx_utils::Material wall_mtl;
  wall_mtl.ambient_color  = glm::vec3(0.5f, 0.5f, 0.5f);
  wall_mtl.diffuse_color  = glm::vec3(0.5f, 0.5f, 0.5f);
  wall_mtl.specular_color = glm::vec3(0.5f, 0.5f, 0.5f);
  wall_mtl.emission_color = glm::vec3(0.f, 0.f, 0.f);
  wall_mtl.shininess = 10;
  wall.material_list.push_back(std::move(wall_mtl));
  meshes.push_back(&wall);

  // // Create F16 mesh
  // std::vector<gfx_utils::Mesh> f16_meshes;
  // if (!gfx_utils::CreateMeshesFromFile(&f16_meshes, "assets/models/f-16.obj")) {
  //   std::cerr << "Failed to load f16 mesh" << std::endl;
  //   exit(1);
  // }
  // for (auto& mesh : f16_meshes) {
  //   meshes.push_back(&mesh);
  // }

  std::vector<gfx_utils::SceneObject*> scene_objs;

  std::vector<gfx_utils::SceneObject> room_children_objs;

  room_children_objs.push_back(
      gfx_utils::SceneObject(glm::vec3(0.f, 0.f, -40.f),
                            glm::vec3(2.f, 1.f, 1.f), 
                            0.f, 0.f, 0.f)); // Front wall
  room_children_objs.push_back(
      gfx_utils::SceneObject(glm::vec3(0.f, 0.f, 40.f),
                             glm::vec3(2.f, 1.f, 1.f), 
                             kPi, 0.f, 0.f)); // Back wall
  room_children_objs.push_back(
      gfx_utils::SceneObject(glm::vec3(-20.f, 0.f, 0.f),
                             glm::vec3(4.f, 1.f, 1.f), 
                             0.5f * kPi, 0.f, 0.f)); // Left wall
  room_children_objs.push_back(
      gfx_utils::SceneObject(glm::vec3(20.f, 0.f, 0.f),
                             glm::vec3(4.f, 1.f, 1.f), 
                             -0.5f * kPi, 0.f, 0.f)); // Right wall
  room_children_objs.push_back(
      gfx_utils::SceneObject(glm::vec3(0.f, 10.f, 0.f),
                             glm::vec3(2.f, 4.f, 1.f), 
                             0.f, 0.5f * kPi, 0.f)); // Ceiling
  room_children_objs.push_back(
      gfx_utils::SceneObject(glm::vec3(0.f, -10.f, 0.f),
                             glm::vec3(2.f, 4.f, 1.f), 
                             0.f, -0.5f * kPi, 0.f)); // Floor  
  // TODO(colintan): Think of a better way to do this
  for (auto& obj : room_children_objs) {
    obj.AddMesh(&wall);
  }

  // Room is a meshless scene object that has the walls, ceiling and floor
  // as its children - so we can transform the whole room as a single object                           
  gfx_utils::SceneObject room_obj(glm::vec3(0.f, 5.f, 0.f),
                                  glm::vec3(1.f, 1.f, 1.f),
                                  0.f, 0.f, 0.f);
  for (auto& obj : room_children_objs) {
    obj.SetParent(&room_obj);
  }

  for (auto& obj : room_children_objs) {
    scene_objs.push_back(&obj);
  }
  scene_objs.push_back(&room_obj);

  // // Create SceneObject for the F16
  // gfx_utils::SceneObject f16_obj;
  // for (auto& mesh : f16_meshes) {
  //   f16_obj.AddMesh(&mesh);
  // }
  // f16_obj.SetScale(glm::vec3(5.f, 5.f, 5.f));
  // scene_objs.push_back(&f16_obj);

  // Initialize mesh and texture for Cube

  // TODO(colintan): How to get a single mesh, not having to pass in a vector
  std::vector<gfx_utils::Mesh> cube_meshes;
  if (!gfx_utils::CreateMeshesFromFile(&cube_meshes, "assets/cube", 
                                       "assets/cube/cube.obj")) {
    std::cerr << "Failed to load cube mesh" << std::endl;
    exit(1);
  }
  meshes.push_back(&cube_meshes[0]);

  gfx_utils::SceneObject cube_obj;
  cube_obj.AddMesh(&cube_meshes[0]);
  cube_obj.SetScale(glm::vec3(5.f, 5.f, 5.f));
  scene_objs.push_back(&cube_obj);

  std::unordered_map<std::string, gfx_utils::Texture> texture_data_map;

  // Load Cube texture
  gfx_utils::Material& cube_mtl = cube_meshes[0].material_list[0];
  texture_data_map[cube_mtl.diffuse_texname] = gfx_utils::Texture();
  if (!gfx_utils::CreateTextureFromFile(
          &texture_data_map[cube_mtl.diffuse_texname], cube_tex_directory,
          cube_meshes[0].material_list[0].diffuse_texname)) {
    std::cerr << "Failed to load cube texture" << std::endl;
    exit(1);        
  }
          
  // Enable all necessary GL settings
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  gfx_utils::Program program;
  if (!program.CreateProgram(vert_shader_path, frag_shader_path)) {
    exit(1);
  }
  GLuint program_id = program.GetProgramId();
  glUseProgram(program.GetProgramId());

  GLuint vao_id;
  glGenVertexArrays(1, &vao_id);

  std::vector<GLuint> pos_vbo_id_list;
  std::vector<GLuint> normal_vbo_id_list;
  std::vector<GLuint> texcoord_vbo_id_list;
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

    GLuint texcoord_vbo_id;
    glGenBuffers(1, &texcoord_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, mesh_ptr->texcoord_data.size() * 2 * sizeof(float),
      &mesh_ptr->texcoord_data[0], GL_STATIC_DRAW);
    texcoord_vbo_id_list.push_back(texcoord_vbo_id);

    if (mesh_ptr->material_list.size() != 0) {
      GLuint mtl_vbo_id;
      glGenBuffers(1, &mtl_vbo_id);
      glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id);
      glBufferData(GL_ARRAY_BUFFER, 
        mesh_ptr->mtl_id_data.size() * sizeof(unsigned int), 
        &mesh_ptr->mtl_id_data[0], 
        GL_STATIC_DRAW);
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
      &mesh_ptr->index_data[0],
      GL_STATIC_DRAW);
    ibo_id_list.push_back(ibo_id);
  }

  std::unordered_map<std::string, GLuint> texture_id_map;

  for (auto it = texture_data_map.begin(); it != texture_data_map.end(); ++it) {
    const std::string& texname = it->first;
    const gfx_utils::Texture& texture = it->second;

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    GLenum format = texture.has_alpha ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, texture.tex_width, 
      texture.tex_height, 0, format, GL_UNSIGNED_BYTE, &texture.tex_data[0]);

    glBindTexture(GL_TEXTURE_2D, 0);

    texture_id_map[texname] = texture_id;
  }

  bool should_quit = false;

  while (!should_quit) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view_mat = camera.CalcViewMatrix();
    glm::mat4 proj_mat = glm::perspective(glm::radians(30.f), 
                                          window.GetAspectRatio(),
                                          0.1f, 10000.f);

    program.GetUniform("camera_pos").Set(camera.GetCameraLocation());

    glm::vec3 light_pos = glm::vec3(view_mat * glm::mat4(1.f) *
                                    glm::vec4(0.f, 10.f, 10.f, 1.f));

    program.GetUniform("lights", 0, "position").Set(light_pos);
    program.GetUniform("lights", 0, "diffuse_intensity")
           .Set(glm::vec3(0.5f, 0.5f, 0.5f));
    program.GetUniform("lights", 0, "specular_intensity")
           .Set(glm::vec3(0.5f, 0.5f, 0.5f));

    glm::vec3 light2_pos = glm::vec3(view_mat * glm::mat4(1.f) *
                                    glm::vec4(0.f, 10.f, -10.f, 1.f));

    program.GetUniform("lights", 1, "position").Set(light2_pos);
    program.GetUniform("lights", 1, "diffuse_intensity")
           .Set(glm::vec3(0.5f, 0.f, 0.f));
    program.GetUniform("lights", 1, "specular_intensity")
           .Set(glm::vec3(0.5f, 0.f, 0.f));                                

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

          if (!mtl.ambient_texname.empty()) {
            program.GetUniform("materials", i, "has_ambient_tex").Set(true);      

            // TODO(colintan): This won't work with multiple textures - the
            // texture unit will only refer to the last texture
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture_id_map[mtl.ambient_texname]);
            program.GetUniform("materials", i, "ambient_texture").Set(1);
          }
          else {
            program.GetUniform("materials", i, "has_ambient_tex").Set(false); 
          }

          if (!mtl.diffuse_texname.empty()) {
            program.GetUniform("materials", i, "has_diffuse_tex").Set(true); 

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture_id_map[mtl.diffuse_texname]);
            program.GetUniform("materials", i, "diffuse_texture").Set(2);
          }
          else {
            program.GetUniform("materials", i, "has_diffuse_tex").Set(false); 
          }

          if (!mtl.specular_texname.empty()) {
            program.GetUniform("materials", i, "has_specular_tex").Set(true); 

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, texture_id_map[mtl.specular_texname]);
            program.GetUniform("materials", i, "specular_texture").Set(3);
          }
          else {
            program.GetUniform("materials", i, "has_specular_tex").Set(false); 
          }
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

        glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id_list[mesh_idx]);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

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

  for (auto it = texture_id_map.begin(); it != texture_id_map.end(); ++it) {
    glDeleteTextures(1, &it->second);
  }

  glDeleteBuffers(static_cast<GLsizei>(ibo_id_list.size()), &ibo_id_list[0]);
  glDeleteBuffers(static_cast<GLsizei>(texcoord_vbo_id_list.size()), &texcoord_vbo_id_list[0]);
  glDeleteBuffers(static_cast<GLsizei>(mtl_vbo_id_list.size()), &mtl_vbo_id_list[0]);
  glDeleteBuffers(static_cast<GLsizei>(normal_vbo_id_list.size()), 
      &normal_vbo_id_list[0]);
  glDeleteBuffers(static_cast<GLsizei>(pos_vbo_id_list.size()), 
      &pos_vbo_id_list[0]);

  glDeleteVertexArrays(1, &vao_id);

  program.DestroyProgram();

  glfwTerminate();

  return 0;
}