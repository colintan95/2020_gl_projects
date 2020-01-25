#ifndef GFX_UTILS_RENDERERS_SIMPLE_RENDERER_H_
#define GFX_UTILS_RENDERERS_SIMPLE_RENDERER_H_

#include "gfx_utils/renderers/renderer.h"

namespace gfx_utils {

class SimpleRenderer : public Renderer {
public:
  bool Initialize() override;
  void Destroy() override;

  void Render(const EntityList& entities) override;

};

} // namespace gfx_utils

#endif // GFX_UTILS_RENDERERS_SIMPLE_RENDERER_H_