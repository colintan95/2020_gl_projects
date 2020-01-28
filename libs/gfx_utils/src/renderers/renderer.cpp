#include "gfx_utils/renderers/renderer.h"

namespace gfx_utils {

void Renderer::SetResourceManager(GLResourceManager* manager) {
  resource_manager_ = manager;
}

void Renderer::SetWindow(Window* window) {
  window_ = window;
}

void Renderer::SetCamera(Camera* camera) {
  camera_ = camera;
}

GLResourceManager* Renderer::GetResourceManager() {
  return resource_manager_;
}

Window* Renderer::GetWindow() {
  return window_;
}

Camera* Renderer::GetCamera() {
  return camera_;
}

} // namespace gfx_utils