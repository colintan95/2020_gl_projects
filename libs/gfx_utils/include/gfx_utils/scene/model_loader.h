#ifndef GFX_UTILS_SCENE_MODEL_LOADER_H_
#define GFX_UTILS_SCENE_MODEL_LOADER_H_

#include "tinyobjloader/tiny_obj_loader.h"

#include <string>
#include <memory>

#include "gfx_utils/model.h"
#include "gfx_utils/mesh.h"

namespace gfx_utils {

class ModelLoader {
public:
  std::shared_ptr<Model> LoadModelFromFile(const std::string& name,
                                           const std::string& mtl_directory,
                                           const std::string& path,
                                           bool indexed = true);

private:
  void LoadVertexData(
      Mesh* mesh, 
      const tinyobj::shape_t& shape,
      const tinyobj::attrib_t& attribs);

  bool LoadVertexDataNoIndex(
      Mesh* mesh,
      const tinyobj::shape_t& shape,
      const tinyobj::attrib_t& attribs);

  void ComputeNormals(Mesh* mesh);

  void LoadMaterialData(
      Mesh* mesh,
      const tinyobj::shape_t& shape,
      const std::vector<tinyobj::material_t>& material_data);

  // Indices by vertex (e.g. in a triangle, we have index 0, 1, 2 for the
  // three vertices)
  // Does NOT check if the index is valid - do the check before calling the
  // function
  glm::vec3 GetPositionAtIndex(
      size_t vert_idx, const tinyobj::attrib_t& attribs);
  glm::vec3 GetNormalAtIndex(
      size_t vert_idx, const tinyobj::attrib_t& attribs);
  glm::vec2 GetTexcoordAtIndex(
      size_t vert_idx, const tinyobj::attrib_t& attribs);
};

} // namespace gfx_utils

#endif // GFX_UTILS_SCENE_MODEL_LOADER_H_