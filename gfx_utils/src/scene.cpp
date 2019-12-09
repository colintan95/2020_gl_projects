#include "gfx_utils/scene.h"

#include "gfx_utils/mesh.h"

#include <glm/gtx/transform.hpp>

namespace gfx_utils {

SceneObject::SceneObject(Mesh *mesh, glm::vec3 location, glm::vec3 scale,
                         float yaw, float pitch, float roll) {
  parent_ = nullptr;

  mesh_ = mesh;
  
  location_ = location;
  scale_ = scale;
  rotation_ = glm::quat(glm::vec3(pitch, yaw, roll));                             
}

SceneObject::SceneObject(Mesh *mesh) {
  SceneObject(mesh, glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f), 
              0.f, 0.f, 0.f);
}

glm::mat4 SceneObject::CalcTransform() const {
  glm::mat4 model_mat = glm::translate(glm::mat4(1.f), location_) *
                        glm::mat4_cast(rotation_) *
                        glm::scale(scale_);

  if (parent_ != nullptr) {
    // TODO(colintan): May be inefficient if a parent has multiple children,
    // and each child is calculating the same parent transform
    model_mat = parent_->CalcTransform() * model_mat;
  }

  return model_mat;
}

void SceneObject::SetParent(SceneObject *parent) {
  parent_ = parent;
}

} // namespace gfx_utils