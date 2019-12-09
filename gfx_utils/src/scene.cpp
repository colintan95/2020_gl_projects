#include "gfx_utils/scene.h"

#include "gfx_utils/mesh.h"

#include <glm/gtx/transform.hpp>

namespace gfx_utils {

glm::mat4 SceneObject::CalcModelMatrix() const {
  return glm::translate(glm::mat4(1.f), location) *
         glm::mat4_cast(rotation) *
         glm::scale(scale);
}

SceneObject CreateSceneObject(Mesh *mesh, glm::vec3 location, glm::vec3 scale,
                              float yaw, float pitch, float roll) {
  SceneObject object;
  
  object.mesh = mesh;
  
  object.location = location;
  object.scale = scale;
  object.rotation = glm::quat(glm::vec3(pitch, yaw, roll));

  return std::move(object);                               
}

SceneObject CreateSceneObject(Mesh *mesh) {
  return CreateSceneObject(mesh, glm::vec3(0.f, 0.f, 0.f), 
                           glm::vec3(1.f, 1.f, 1.f), 0.f, 0.f, 0.f);
}

} // namespace gfx_utils