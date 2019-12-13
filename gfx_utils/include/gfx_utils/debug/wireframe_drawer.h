#ifndef GFX_UTILS_WIREFRAME_DRAWER_H_
#define GFX_UTILS_WIREFRAME_DRAWER_H_

#include <GL/glew.h>
#if defined(WIN32)
#include <GL/gl.h>
#endif

#include <glm/matrix.hpp>

#include "gfx_utils/program.h"

namespace gfx_utils {

// Forward declaration
struct Mesh;

class WireframeDrawer {
public:
  bool Initialize();
  void Destroy();

  void Draw(const Mesh* mesh, GLuint pos_vbo_id, GLuint ibo_id, 
            const glm::mat4& mvp_mat);

private:
  Program wireframe_program_;

  GLuint wireframe_vao_;

  bool is_initialized_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_WIREFRAME_DRAWER_H_