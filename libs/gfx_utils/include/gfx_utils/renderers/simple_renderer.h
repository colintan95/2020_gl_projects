#ifndef GFX_UTILS_RENDERERS_SIMPLE_RENDERER_H_
#define GFX_UTILS_RENDERERS_SIMPLE_RENDERER_H_

#include <GL/glew.h>
#if defined(WIN32)
#include <GL/gl.h>
#endif

#include "gfx_utils/renderers/renderer.h"
#include "gfx_utils/program.h"

namespace gfx_utils {

class SimpleRenderer : public Renderer {
public:
  bool Initialize() override;
  void Destroy() override;

  void Render(const EntityList& entities) override;

private:
  void SetTransformUniforms_Mesh(gfx_utils::Mesh& mesh,
                                 glm::mat4& model_mat,
                                 glm::mat4& view_mat,
                                 glm::mat4& proj_mat);
  void SetMaterialUniforms_Mesh(gfx_utils::Mesh& mesh);

private:
  Program program_;

  GLuint vao_id_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_RENDERERS_SIMPLE_RENDERER_H_