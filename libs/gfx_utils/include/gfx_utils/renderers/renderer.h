#ifndef GFX_UTILS_RENDERERS_RENDERER_H_
#define GFX_UTILS_RENDERERS_RENDERER_H_

#include "gfx_utils/resource/resource_manager.h"
#include "gfx_utils/window/camera.h"

namespace gfx_utils {

class Renderer {
public:
  virtual bool Initialize() = 0;
  virtual void Destroy() = 0;

  virtual void Render(const EntityList& entities) = 0;

  void AttachCamera(Camera* camera);

  Camera* GetCamera();

private:
  Camera* camera_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_RENDERERS_RENDERER_H_