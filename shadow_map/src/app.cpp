#include "app.h"

#include <iostream>
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "gfx_utils/primitives.h"

static const float kPi = 3.14159265358979323846f;

static const int kWindowWidth = 1920;
static const int kWindowHeight = 1080;

static const std::string vert_shader_path = "shaders/simple_light.vert";
static const std::string frag_shader_path = "shaders/simple_light.frag";

static const std::string cube_tex_directory = "assets/cube";

static const int kShadowTexWidth = 1024;
static const int kShadowTexHeight = 1024;

void App::Run() {
  Startup();

  MainLoop();

  Cleanup();
}

void App::MainLoop() {
  bool should_quit = false;

  while (!should_quit) {
    ShadowPass();

    LightPass();

    window_.SwapBuffers();
    window_.TickMainLoop();

    if (window_.ShouldQuit()) {
      should_quit = true;
    }
  }
}


void App::ShadowPass() {
  for (int i = 0; i < spotlights_.size(); ++i) {
    auto light_ptr = spotlights_[i];

    glUseProgram(shadow_pass_program_.GetProgramId());
    glViewport(0, 0, kShadowTexWidth, kShadowTexHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_id_list_[i]);
    glClear(GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(shadow_pass_vao_id_);

    const auto& entities = resource_manager_.GetEntities();

    for (auto entity_ptr : entities) {
      if (!entity_ptr->HasModel()) {
        continue;
      }

      for (auto mesh: entity_ptr->GetModel()->GetMeshes()) {
        glm::mat4 model_mat = entity_ptr->CalcTransform();
        glm::mat4 view_mat = 
            glm::lookAt(light_ptr->position,
                        light_ptr->position + light_ptr->direction,
                        light_ptr->camera_up);
        glm::mat4 proj_mat = glm::perspective(light_ptr->cone_angle,
                                              1.f, 5.f, 30.f);
        glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;

        shadow_pass_program_.GetUniform("mvp_mat").Set(mvp_mat);

        int mesh_idx = mesh_to_idx_map_[mesh.id];

        glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id_list_[mesh_idx]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id_list_[mesh_idx]);
        glDrawElements(GL_TRIANGLES, mesh.num_verts, GL_UNSIGNED_INT,
                        (void*)0);
      }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  glBindVertexArray(0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void App::LightPass() {
  glUseProgram(light_pass_program_.GetProgramId());
  glViewport(0, 0, kWindowWidth, kWindowHeight);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view_mat = camera_.CalcViewMatrix();
  glm::mat4 proj_mat = glm::perspective(glm::radians(30.f),
                                        window_.GetAspectRatio(),
                                        0.1f, 1000.f);

  light_pass_program_.GetUniform("camera_pos").Set(camera_.GetCameraLocation());

  light_pass_program_.GetUniform("ambient_intensity")
                     .Set(glm::vec3(0.5f, 0.5f, 0.5f));

  const auto& entities = resource_manager_.GetEntities();

  for (auto entity_ptr : entities) {
    if (!entity_ptr->HasModel()) {
      continue;
    }

    for (auto& mesh : entity_ptr->GetModel()->GetMeshes()) {
      glm::mat4 model_mat = entity_ptr->CalcTransform();
      
      LightPass_SetTransformUniforms_Mesh(mesh, model_mat, view_mat, proj_mat);

      LightPass_SetMaterialUniforms_Mesh(mesh);

      LightPass_SetLightUniforms_Mesh(mesh, model_mat, view_mat);

      // Set vertex attributes

      glBindVertexArray(light_pass_vao_id_);

      int mesh_idx = mesh_to_idx_map_[mesh.id];

      glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id_list_[mesh_idx]);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id_list_[mesh_idx]);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id_list_[mesh_idx]);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      if (mesh.material_list.size() != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id_list_[mesh_idx]);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, 0, (GLvoid*)0);
      }
      else {
        // TODO(colintan): Disable materials for this mesh
      }

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id_list_[mesh_idx]);
      glDrawElements(GL_TRIANGLES, mesh.num_verts, GL_UNSIGNED_INT,
          (void*)0);
    }
  }

  glBindVertexArray(0);
}

void App::LightPass_SetTransformUniforms_Mesh(gfx_utils::Mesh& mesh,
                                              glm::mat4& model_mat,
                                              glm::mat4& view_mat,
                                              glm::mat4& proj_mat) {
  glm::mat4 mv_mat = view_mat * model_mat;
  glm::mat4 mvp_mat = proj_mat * mv_mat;

  light_pass_program_.GetUniform("mv_mat").Set(mv_mat);
  light_pass_program_.GetUniform("mvp_mat").Set(mvp_mat);

  // TODO(colintan): Check that this is computing correctly
  glm::mat3 normal_mat =
      glm::mat3(glm::transpose(glm::inverse(mv_mat)));
  light_pass_program_.GetUniform("normal_mat").Set(normal_mat);                                              
}

void App::LightPass_SetMaterialUniforms_Mesh(gfx_utils::Mesh& mesh) {
  // TODO(colintan): Modify this so that we can render a mesh that has
  // multiple materials
  const auto& mtl_list = mesh.material_list;
  for (int i = 0; i < mtl_list.size(); ++i) {
    const auto& mtl = mtl_list[i];

    light_pass_program_.GetUniform("materials", i, "ambient_color")
                        .Set(mtl.ambient_color);
    light_pass_program_.GetUniform("materials", i, "diffuse_color")
                        .Set(mtl.diffuse_color);
    light_pass_program_.GetUniform("materials", i, "specular_color")
                        .Set(mtl.specular_color);
    light_pass_program_.GetUniform("materials", i, "emission_color")
                        .Set(mtl.emission_color);
    light_pass_program_.GetUniform("materials", i, "shininess")
            .Set(mtl.shininess);

    if (!mtl.ambient_texname.empty()) {
      light_pass_program_.GetUniform("materials", i, "has_ambient_tex")
                          .Set(true);

      // TODO(colintan): This won't work with multiple textures - the
      // texture unit will only refer to the last texture
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, texture_id_map_[mtl.ambient_texname]);
      light_pass_program_.GetUniform("materials", i, "ambient_texture")
                          .Set(1);
    }
    else {
      light_pass_program_.GetUniform("materials", i, "has_ambient_tex")
                          .Set(false);
    }

    if (!mtl.diffuse_texname.empty()) {
      light_pass_program_.GetUniform("materials", i, "has_diffuse_tex")
                          .Set(true);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, texture_id_map_[mtl.diffuse_texname]);
      light_pass_program_.GetUniform("materials", i, "diffuse_texture")
                          .Set(2);
    }
    else {
      light_pass_program_.GetUniform("materials", i, "has_diffuse_tex")
                          .Set(false);
    }

    if (!mtl.specular_texname.empty()) {
      light_pass_program_.GetUniform("materials", i, "has_specular_tex")
                          .Set(true);

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, texture_id_map_[mtl.specular_texname]);
      light_pass_program_.GetUniform("materials", i, "specular_texture")
                          .Set(3);
    }
    else {
      light_pass_program_.GetUniform("materials", i, "has_specular_tex")
                          .Set(false);
    }
  }
}

void App::LightPass_SetLightUniforms_Mesh(gfx_utils::Mesh& mesh,
                                          glm::mat4& model_mat,
                                          glm::mat4& view_mat) {
  for (int i = 0; i < spotlights_.size(); ++i) {  
    auto light_ptr = spotlights_[i];

    light_pass_program_.GetUniform("lights", i, "is_active").Set(1);

    // Position of light in modelview space
    glm::vec3 pos_mv =
        glm::vec3(view_mat * glm::vec4(light_ptr->position, 1.f));
    glm::vec3 dir_mv =
        glm::vec3(glm::mat3(glm::transpose(glm::inverse(view_mat))) *
                  light_ptr->direction);

    light_pass_program_.GetUniform("lights", i, "position_mv")
                        .Set(pos_mv);
    light_pass_program_.GetUniform("lights", i, "diffuse_intensity")
                        .Set(light_ptr->diffuse_intensity);
    light_pass_program_.GetUniform("lights", i, "specular_intensity")
                        .Set(light_ptr->specular_intensity);
    light_pass_program_.GetUniform("lights", i, "direction_mv")
                        .Set(dir_mv);
    light_pass_program_.GetUniform("lights", i, "cone_angle")
                        .Set(light_ptr->cone_angle);

    // // TODO(colintan): Don't hardcode this
    glActiveTexture(GL_TEXTURE10 + i);
    glBindTexture(GL_TEXTURE_2D, shadow_tex_id_list_[i]);

    light_pass_program_.GetUniform("lights", i, "shadow_tex").Set(10 + i);

    glm::mat4 light_view_mat =
      glm::lookAt(light_ptr->position,
                  light_ptr->position + light_ptr->direction,
                  light_ptr->camera_up);
    glm::mat4 light_proj_mat = glm::perspective(light_ptr->cone_angle,
                                                1.f, 5.f, 30.f);

    glm::mat4 shadow_mat = light_proj_mat * light_view_mat *
        model_mat;

    light_pass_program_.GetUniform("shadow_mats", i).Set(shadow_mat);
  }
}

void App::Startup() {
  if (!window_.Inititalize(kWindowWidth, kWindowHeight, "Shadow Map")) {
    std::cerr << "Failed to initialize gfx window" << std::endl;
    exit(1);
  }

  if (!camera_.Initialize(&window_)) {
    std::cerr << "Failed to initialize camera" << std::endl;
    exit(1);
  }

  resource_manager_.LoadResourcesFromJson("assets/resources.json");

  // Add custom room entity
  auto room_model_ptr = std::make_shared<gfx_utils::Model>("room");
  room_model_ptr->GetMeshes() = 
      std::move(gfx_utils::CreateRoom(30.f, 20.f, 80.f));
  auto room_entity_ptr = std::make_shared<gfx_utils::Entity>("room");
  room_entity_ptr->SetModel(room_model_ptr);
  resource_manager_.AddEntity(room_entity_ptr);

  const auto& models = resource_manager_.GetModels();
  const auto& entities = resource_manager_.GetEntities();

  spotlights_ = resource_manager_.GetLightsByType<gfx_utils::Spotlight>();

  // Enable all necessary GL settings
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  if (!light_pass_program_.CreateProgram(vert_shader_path, frag_shader_path)) {
    std::cerr << "Could not create light pass program." << std::endl;
    exit(1);
  }
  glUseProgram(light_pass_program_.GetProgramId());

  glGenVertexArrays(1, &light_pass_vao_id_);

  for (auto model_ptr : models) {
    for (gfx_utils::Mesh& mesh : model_ptr->GetMeshes()) {
      gfx_utils::MeshId id = mesh.id;

      mesh_to_idx_map_[id] = static_cast<int>(ibo_id_list_.size());

      GLuint pos_vbo_id;
      glGenBuffers(1, &pos_vbo_id); // TODO(colintan): Is bulk allocating faster?
      glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
      glBufferData(GL_ARRAY_BUFFER, mesh.pos_data.size() * 3 * sizeof(float),
                  &mesh.pos_data[0], GL_STATIC_DRAW);
      pos_vbo_id_list_.push_back(pos_vbo_id);

      GLuint normal_vbo_id;
      glGenBuffers(1, &normal_vbo_id);
      glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id);
      glBufferData(GL_ARRAY_BUFFER, mesh.normal_data.size() * 3 * sizeof(float),
                  &mesh.normal_data[0], GL_STATIC_DRAW);
      normal_vbo_id_list_.push_back(normal_vbo_id);

      GLuint texcoord_vbo_id;
      glGenBuffers(1, &texcoord_vbo_id);
      glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id);
      glBufferData(GL_ARRAY_BUFFER, mesh.texcoord_data.size() * 2 * sizeof(float),
                  &mesh.texcoord_data[0], GL_STATIC_DRAW);
      texcoord_vbo_id_list_.push_back(texcoord_vbo_id);

      if (mesh.material_list.size() != 0) {
        GLuint mtl_vbo_id;
        glGenBuffers(1, &mtl_vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id);
        glBufferData(GL_ARRAY_BUFFER,
                    mesh.mtl_id_data.size() * sizeof(unsigned int),
                    &mesh.mtl_id_data[0], GL_STATIC_DRAW);
        mtl_vbo_id_list_.push_back(mtl_vbo_id);
      }
      else {
        mtl_vbo_id_list_.push_back(0);
      }

      GLuint ibo_id;
      glGenBuffers(1, &ibo_id);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   mesh.index_data.size() * sizeof(uint32_t),
                   &mesh.index_data[0], GL_STATIC_DRAW);
      ibo_id_list_.push_back(ibo_id);
    }
  }

  auto& texture_name_map = resource_manager_.GetTextureNameMap();

  for (auto it = texture_name_map.begin(); it != texture_name_map.end(); ++it) {
    const std::string& texname = it->first;
    const auto& texture = it->second;

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = texture->has_alpha ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, texture->tex_width,
                 texture->tex_height, 0, format, GL_UNSIGNED_BYTE,
                 &texture->tex_data[0]);

    glBindTexture(GL_TEXTURE_2D, 0);

    texture_id_map_[texname] = texture_id;
  }

  if (!shadow_pass_program_.CreateProgram("shaders/shadow_pass.vert",
                                          "shaders/shadow_pass.frag")) {
    std::cerr << "Could not create shadow pass program." << std::endl;
    exit(1);
  }
  glUseProgram(shadow_pass_program_.GetProgramId());

  for (auto light_ptr : spotlights_) {
    GLuint shadow_tex_id;
    glGenTextures(1, &shadow_tex_id);
    glBindTexture(GL_TEXTURE_2D, shadow_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kShadowTexWidth,
                  kShadowTexHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    shadow_tex_id_list_.push_back(shadow_tex_id);

    GLuint shadow_fbo_id;
    glGenFramebuffers(1, &shadow_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           shadow_tex_id, 0);
    // TODO(colintan): Possible to remove?
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shadow_fbo_id_list_.push_back(shadow_fbo_id);
  }

  glGenVertexArrays(1, &shadow_pass_vao_id_);
}

void App::Cleanup() {
  glDeleteVertexArrays(1, &shadow_pass_vao_id_);
  
  glDeleteFramebuffers(static_cast<GLsizei>(shadow_fbo_id_list_.size()),
                       &shadow_fbo_id_list_[0]);

  glDeleteTextures(static_cast<GLsizei>(shadow_tex_id_list_.size()),
                   &shadow_tex_id_list_[0]);

  shadow_pass_program_.DestroyProgram();

  for (auto it = texture_id_map_.begin(); it != texture_id_map_.end(); ++it) {
    glDeleteTextures(1, &it->second);
  }

  glDeleteBuffers(static_cast<GLsizei>(ibo_id_list_.size()), &ibo_id_list_[0]);
  glDeleteBuffers(static_cast<GLsizei>(mtl_vbo_id_list_.size()),
                  &mtl_vbo_id_list_[0]);
  glDeleteBuffers(static_cast<GLsizei>(texcoord_vbo_id_list_.size()),
                  &texcoord_vbo_id_list_[0]);
  glDeleteBuffers(static_cast<GLsizei>(normal_vbo_id_list_.size()),
                  &normal_vbo_id_list_[0]);
  glDeleteBuffers(static_cast<GLsizei>(pos_vbo_id_list_.size()),
                  &pos_vbo_id_list_[0]);

  glDeleteVertexArrays(1, &light_pass_vao_id_);
  
  light_pass_program_.DestroyProgram();

  window_.Destroy();
}