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
    renderer_.Render(scene_.GetEntities());

    window_.SwapBuffers();
    window_.TickMainLoop();

    if (window_.ShouldQuit()) {
      should_quit = true;
    }
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
      std::move(gfx_utils::CreateRoom(30.f, 20.f, 80.f));
  auto room_entity_ptr = std::make_shared<gfx_utils::Entity>("room");
  room_entity_ptr->SetModel(room_model_ptr);
  scene_.AddEntity(room_entity_ptr);

  gl_resource_manager_.SetScene(&scene_);

  gl_resource_manager_.CreateGLResources();

  renderer_.Initialize();

  renderer_.SetResourceManager(&gl_resource_manager_);
  renderer_.SetWindow(&window_);
  renderer_.SetCamera(&camera_);
}

void App::Cleanup() {
  renderer_.Destroy();

  gl_resource_manager_.Cleanup();

  window_.Destroy();
}