#ifndef GFX_UTILS_SCENE_H_
#define GFX_UTILS_SCENE_H_

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

namespace gfx_utils {

// Foward declaration
struct Mesh;

class SceneObject {

public:
  SceneObject(Mesh *mesh, glm::vec3 location, glm::vec3 scale,
                    float yaw, float pitch, float roll);
  SceneObject(Mesh *mesh);

  glm::mat4 CalcTransform() const;

  void SetParent(SceneObject *parent);

  const Mesh * GetMesh() const { return mesh_; }

  bool HasMesh() const { return mesh_ != nullptr; }

private:
  SceneObject *parent_;

  Mesh *mesh_;

  glm::vec3 location_;
  glm::quat rotation_;
  glm::vec3 scale_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_SCENE_H_