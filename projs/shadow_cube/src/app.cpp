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

static const std::string kLightPassVertShaderPath = "shaders/simple_light.vert";
static const std::string kLightPassFragShaderPath = "shaders/simple_light.frag";

static const std::string kShadowPassVertShaderPath = "shaders/shadow_pass.vert";
static const std::string kShadowPassFragShaderPath = "shaders/shadow_pass.frag";
static const std::string kShadowPassGeomShaderPath = "shaders/shadow_pass.geom";

static const int kShadowTexWidth = 1024;
static const int kShadowTexHeight = 1024;

static const float kShadowNearPlane = 1.f;
static const float kShadowFarPlane = 25.f;

static const glm::vec3 cubemap_dirs[] = {
  {1.f, 0.f, 0.f}, {-1.f, 0.f, 0.f},
  {0.f, 1.f, 0.f}, {0.f, -1.f, 0.f},
  {0.f, 0.f, 1.f}, {0.f, 0.f, -1.f}
};

static const glm::vec3 cubemap_up[] = {
  {0.f, -1.f, 0.f}, {0.f, -1.f, 0.f},
  {0.f, 0.f, 1.f}, {0.f, 0.f, -1.f},
  {0.f, -1.f, 0.f}, {0.f, -1.f, 0.f}
};

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
  for (int i = 0; i < lights_.size(); ++i) {
    auto light_ptr = lights_[i];

    glUseProgram(shadow_pass_program_.GetProgramId());
    glViewport(0, 0, kShadowTexWidth, kShadowTexHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_id_list_[i]);
    glClear(GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(shadow_pass_vao_id_);

    float near_plane = kShadowNearPlane;
    float far_plane = kShadowFarPlane;

    shadow_pass_program_.GetUniform("light_post").Set(light_ptr->position);
    shadow_pass_program_.GetUniform("far_plane").Set(far_plane);

    const auto& entities = scene_.GetEntities();

    for (auto entity_ptr : entities) {
      if (!entity_ptr->HasModel()) {
        continue;
      }

      for (auto mesh: entity_ptr->GetModel()->GetMeshes()) {
        glm::mat4 model_mat = entity_ptr->ComputeTransform();

        shadow_pass_program_.GetUniform("model_mat").Set(model_mat);

        glm::mat4 proj_mat = glm::perspective(glm::radians(90.f), 1.f,
                                              near_plane, far_plane);

        // TODO(colintan): Figure out why the camera ups must have these vals
        for (int i = 0; i < 6; ++i) {
          glm::mat4 view_mat = 
              glm::lookAt(light_ptr->position,
                          light_ptr->position + cubemap_dirs[i],
                          cubemap_up[i]);
          glm::mat4 shadow_mat = proj_mat * view_mat;

          shadow_pass_program_.GetUniform("shadow_mats", i).Set(shadow_mat);
        }

        GLuint pos_vbo_id = 
            resource_manager_.GetMeshVboId(mesh.id, 
                                          gfx_utils::kVertTypePosition);
        glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

        GLuint ibo_id = resource_manager_.GetMeshIboId(mesh.id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
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

  light_pass_program_.GetUniform("camera_pos")
                     .Set(camera_.GetCameraLocation());

  light_pass_program_.GetUniform("ambient_intensity")
                     .Set(glm::vec3(0.5f, 0.5f, 0.5f));

  const auto& entities = scene_.GetEntities();

  for (auto entity_ptr : entities) {
    if (!entity_ptr->HasModel()) {
      continue;
    }

    for (auto& mesh : entity_ptr->GetModel()->GetMeshes()) {
      glm::mat4 model_mat = entity_ptr->ComputeTransform();
      
      LightPass_SetTransformUniforms_Mesh(mesh, model_mat, view_mat, proj_mat);

      LightPass_SetMaterialUniforms_Mesh(mesh);

      LightPass_SetLightUniforms_Mesh(mesh);

      // Set vertex attributes

      glBindVertexArray(light_pass_vao_id_);

      GLuint pos_vbo_id = 
          resource_manager_.GetMeshVboId(mesh.id, 
                                         gfx_utils::kVertTypePosition);
      glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      GLuint normal_vbo_id =
          resource_manager_.GetMeshVboId(mesh.id, 
                                         gfx_utils::kVertTypeNormal);
      glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      GLuint texcoord_vbo_id =
          resource_manager_.GetMeshVboId(mesh.id, 
                                         gfx_utils::kVertTypeTexcoord);
      glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      GLuint mtl_vbo_id =
          resource_manager_.GetMeshVboId(mesh.id, 
                                         gfx_utils::kVertTypeMtlId);
      glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id);
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, 0, (GLvoid*)0);

      GLuint ibo_id = resource_manager_.GetMeshIboId(mesh.id);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);

      glDrawElements(GL_TRIANGLES, mesh.num_verts, GL_UNSIGNED_INT, (void*)0);
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

  light_pass_program_.GetUniform("model_mat").Set(model_mat);
  light_pass_program_.GetUniform("mv_mat").Set(mv_mat);
  light_pass_program_.GetUniform("mvp_mat").Set(mvp_mat);                                    
}

void App::LightPass_SetMaterialUniforms_Mesh(gfx_utils::Mesh& mesh) {
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

      glActiveTexture(GL_TEXTURE1);
      GLuint tex_gl_id = 
          resource_manager_.GetTextureId(mtl.ambient_texname);
      glBindTexture(GL_TEXTURE_2D, tex_gl_id);
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
      GLuint tex_gl_id = 
          resource_manager_.GetTextureId(mtl.diffuse_texname);
      glBindTexture(GL_TEXTURE_2D, tex_gl_id);
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
      GLuint tex_gl_id = 
          resource_manager_.GetTextureId(mtl.specular_texname);
      glBindTexture(GL_TEXTURE_2D, tex_gl_id);
      light_pass_program_.GetUniform("materials", i, "specular_texture")
                         .Set(3);
    }
    else {
      light_pass_program_.GetUniform("materials", i, "has_specular_tex")
                         .Set(false);
    }
  }
}

void App::LightPass_SetLightUniforms_Mesh(gfx_utils::Mesh& mesh) {

  for (int i = 0; i < lights_.size(); ++i) {  
    auto light_ptr = lights_[i];

    light_pass_program_.GetUniform("lights", i, "is_active").Set(1);

    light_pass_program_.GetUniform("lights", i, "position")
                       .Set(light_ptr->position);
    light_pass_program_.GetUniform("lights", i, "diffuse_intensity")
                       .Set(light_ptr->diffuse_intensity);
    light_pass_program_.GetUniform("lights", i, "specular_intensity")
                       .Set(light_ptr->specular_intensity);

    // // TODO(colintan): Don't hardcode this
    glActiveTexture(GL_TEXTURE10 + i);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_tex_id_list_[i]);

    light_pass_program_.GetUniform("lights", i, "shadow_tex").Set(10 + i);

    light_pass_program_.GetUniform("lights", i, "far_plane")
                       .Set(kShadowFarPlane);
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

  scene_.LoadSceneFromJson("assets/scene.json");

  // Add custom room entity
  auto room_model_ptr = std::make_shared<gfx_utils::Model>("room");
  room_model_ptr->GetMeshes() = 
      std::move(gfx_utils::CreateRoom(10.f, 5.f, 10.f));
  auto room_entity_ptr = std::make_shared<gfx_utils::Entity>("room");
  room_entity_ptr->SetModel(room_model_ptr);
  scene_.AddEntity(room_entity_ptr);

  resource_manager_.SetScene(&scene_);

  resource_manager_.CreateGLResources();

  lights_ = scene_.GetLightsByType<gfx_utils::PointLight>();

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  if (!light_pass_program_.CreateFromFiles(kLightPassVertShaderPath, 
                                           kLightPassFragShaderPath)) {
    std::cerr << "Could not create light pass program." << std::endl;
    exit(1);
  }
  glUseProgram(light_pass_program_.GetProgramId());

  glGenVertexArrays(1, &light_pass_vao_id_);

  if (!shadow_pass_program_.CreateFromFiles(kShadowPassVertShaderPath,
                                            kShadowPassFragShaderPath,
                                            kShadowPassGeomShaderPath)) {
    std::cerr << "Could not create shadow pass program." << std::endl;
    exit(1);
  }
  glUseProgram(shadow_pass_program_.GetProgramId());

  for (auto light_ptr : lights_) {
    GLuint shadow_tex_id;
    glGenTextures(1, &shadow_tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_tex_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < 6; ++i) {   
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                   kShadowTexWidth, kShadowTexHeight, 0, GL_DEPTH_COMPONENT, 
                   GL_FLOAT, NULL);
    }

    shadow_tex_id_list_.push_back(shadow_tex_id);

    GLuint shadow_fbo_id;
    glGenFramebuffers(1, &shadow_fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo_id);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_tex_id, 0);
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

  shadow_pass_program_.Destroy();

  glDeleteVertexArrays(1, &light_pass_vao_id_);
  
  light_pass_program_.Destroy();

  resource_manager_.Cleanup();

  window_.Destroy();
}