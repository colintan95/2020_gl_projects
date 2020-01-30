#ifndef GFX_UTILS_RENDERERS_RENDERER_H_
#define GFX_UTILS_RENDERERS_RENDERER_H_

#include "gfx_utils/window/camera.h"
#include "gfx_utils/window/window.h"
#include "gfx_utils/gl/gl_resource_manager.h"

namespace gfx_utils {

class Renderer {
public:
  virtual bool Initialize() = 0;
  virtual void Destroy() = 0;

  virtual void Render(const EntityList& entities) = 0;

  void SetResourceManager(GLResourceManager* manager);
  void SetWindow(Window* window);
  void SetCamera(Camera* camera);

  GLResourceManager* GetResourceManager();
  Window* GetWindow();
  Camera* GetCamera();

private:
  GLResourceManager* resource_manager_;
  Window* window_;
  Camera* camera_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_RENDERERS_RENDERER_H_