#ifndef GFX_UTILS_GL_GL_RESOURCE_MANAGER_H_
#define GFX_UTILS_GL_GL_RESOURCE_MANAGER_H_

#include <unordered_map>

#include <GL/glew.h>
#if defined(WIN32)
#include <GL/gl.h>
#endif

#include "gfx_utils/mesh.h"
#include "gfx_utils/texture.h"

namespace gfx_utils {

enum VertType {
  kVertTypePosition,
  kVertTypeNormal,
  kVertTypeTexcoord,
  kVertTypeMtlId
};

class GLResourceManager {
public:
  void Cleanup();

  void CreateMeshResources(const Mesh& mesh);
  void CreateTextureResources(const Texture& texture);

  GLuint GetMeshVboId(MeshId id, VertType vert_type);
  GLuint GetMeshIboId(MeshId id);
  GLuint GetTextureId(TextureId id);

private:
  std::unordered_map<TextureId, GLuint> texture_gl_id_map_;

  std::unordered_map<MeshId, GLuint> mesh_pos_gl_id_map_;
  std::unordered_map<MeshId, GLuint> mesh_normal_gl_id_map_;
  std::unordered_map<MeshId, GLuint> mesh_texcoord_gl_id_map_;
  std::unordered_map<MeshId, GLuint> mesh_mtl_id_gl_id_map_;

  std::unordered_map<MeshId, GLuint> mesh_ibo_gl_id_map_;
};

} // namespace gfx_utils

#endif //GFX_UTILS_GL_GL_RESOURCE_MANAGER_H_