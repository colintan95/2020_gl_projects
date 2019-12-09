#ifndef GFX_UTILS_SCENE_H_
#define GFX_UTILS_SCENE_H_

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

namespace gfx_utils {

// Foward declaration
struct Mesh;

struct SceneObject {
  Mesh *mesh;

  glm::vec3 location;
  glm::quat rotation;
  glm::vec3 scale;

  glm::mat4 CalcModelMatrix() const;
};

SceneObject CreateSceneObject(Mesh *mesh, glm::vec3 location, glm::vec3 scale,
                              float yaw, float pitch, float roll);

SceneObject CreateSceneObject(Mesh *mesh);

} // namespace gfx_utils

#endif // GFX_UTILS_SCENE_H_