#include "gfx_utils/renderers/renderer.h"

namespace gfx_utils {

void Renderer::AttachCamera(Camera* camera) {
  camera_ = camera;
}

Camera* Renderer::GetCamera() {
  return camera_;
}

} // namespace gfx_utils