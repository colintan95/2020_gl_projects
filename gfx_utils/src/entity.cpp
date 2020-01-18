#include "gfx_utils/entity.h"

#include <cassert>

#include "gfx_utils/mesh.h"

#include <glm/gtx/transform.hpp>

namespace gfx_utils {

Entity::Entity() {
  parent_ = nullptr;
  
  location_ = glm::vec3(0.f, 0.f, 0.f);
  scale_ = glm::vec3(1.f, 1.f, 1.f);
  rotation_ = glm::quat(glm::vec3(0.f, 0.f, 0.f));
}

glm::mat4 Entity::CalcTransform() const {
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

void Entity::SetParent(std::shared_ptr<Entity> parent) {
  parent_ = parent;
}

void Entity::SetModel(std::shared_ptr<Model> model) {
  model_ = model;
}

void Entity::SetLocation(glm::vec3 location) {
  location_ = location;
}

void Entity::SetScale(glm::vec3 scale) {
  scale_ = scale;
}

void Entity::SetRotation(float yaw, float pitch, float roll) {
  rotation_ = glm::quat(glm::vec3(pitch, yaw, roll));    
}

std::shared_ptr<Model> Entity::GetModel() { 
  assert(model_ != nullptr);
  return model_;
}

bool Entity::HasModel() const {
  return model_ != nullptr;
}

} // namespace gfx_utils