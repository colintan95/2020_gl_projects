#include "app.h"

#include <iostream>
#include <cstdlib>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "gfx_utils/primitives.h"

//
// Significant portion of implementation adapted from: LearnOpenGL
//   https://learnopengl.com/Advanced-Lighting/SSAO
//

static const float kPi = 3.14159265358979323846f;

static const int kWindowWidth = 1920;
static const int kWindowHeight = 1080;

static const std::string kReflectPassVertShaderPath = "shaders/reflection.vert";
static const std::string kReflectPassFragShaderPath = "shaders/reflection.frag";

// Format of vertex - {pos_x, pos_y, pos_z, texcoord_u, texcoord_v}
static const float kQuadVertices[] = {
  -1.f,  1.f, 0.f, 0.f, 1.f,
  -1.f, -1.f, 0.f, 0.f, 0.f,
   1.f,  1.f, 0.f, 1.f, 1.f,
   1.f, -1.f, 0.f, 1.f, 0.f
};

void App::Run() {
  Startup();

  MainLoop();

  Cleanup();
}

void App::MainLoop() {
  bool should_quit = false;

  while (!should_quit) {

    ReflectPass();

    window_.SwapBuffers();
    window_.TickMainLoop();

    if (window_.ShouldQuit()) {
      should_quit = true;
    }
  }
}

void App::ReflectPass() {
  glUseProgram(reflect_pass_program_.GetProgramId());
  glViewport(0, 0, kWindowWidth, kWindowHeight);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(reflect_pass_vao_);

  glm::mat4 view_mat = camera_.CalcViewMatrix();
  glm::mat4 proj_mat = glm::perspective(glm::radians(30.f),
                                        window_.GetAspectRatio(),
                                        0.1f, 1000.f);    

  const auto& entities = scene_.GetEntities();   

  for (auto entity_ptr : entities) {
    if (!entity_ptr->HasModel()) {
      continue;
    }

    for (auto& mesh : entity_ptr->GetModel()->GetMeshes()) {
      glm::mat4 model_mat = entity_ptr->ComputeTransform();

      glm::mat4 mv_mat = view_mat * model_mat;
      glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;
      glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(mv_mat)));
      
      reflect_pass_program_.GetUniform("mv_mat").Set(mv_mat);
      reflect_pass_program_.GetUniform("mvp_mat").Set(mvp_mat);
      reflect_pass_program_.GetUniform("normal_mat").Set(normal_mat);

      reflect_pass_program_.GetUniform("camera_pos")
                           .Set(camera_.GetCameraLocation());

      GLuint cubemap_id = resource_manager_.GetCubemapId("skybox");
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);
      reflect_pass_program_.GetUniform("cubemap").Set(1);

      // Set vertex attributes

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

      glDrawArrays(GL_TRIANGLES, 0, mesh.num_verts);
    }
  }
 
  glBindVertexArray(0);

  glUseProgram(0);
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

  scene_.LoadSceneFromJson("scene/scene.json");

  resource_manager_.SetScene(&scene_);

  resource_manager_.CreateGLResources();

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  SetupReflectPass();
}

void App::SetupReflectPass() {
  if (!reflect_pass_program_.CreateFromFiles(kReflectPassVertShaderPath,
                                             kReflectPassFragShaderPath)) {
    std::cerr << "Could not create reflect pass program." << std::endl;
    exit(1);                
  }

  glGenVertexArrays(1, &reflect_pass_vao_);
}

void App::Cleanup() {
  glDeleteVertexArrays(1, &reflect_pass_vao_);

  resource_manager_.Cleanup();

  window_.Destroy();
}