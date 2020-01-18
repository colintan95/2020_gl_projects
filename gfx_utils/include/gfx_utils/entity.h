#ifndef GFX_UTILS_ENTITY_H_
#define GFX_UTILS_ENTITY_H_

#include <vector>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

#include "mesh.h"
#include "model.h"

namespace gfx_utils {

class Entity {

public:
  Entity();

  glm::mat4 CalcTransform() const;

  void SetModel(std::shared_ptr<Model> model);
  void SetParent(std::shared_ptr<Entity> parent);

  void SetLocation(glm::vec3 location);
  void SetScale(glm::vec3 scale);
  void SetRotation(float yaw, float pitch, float roll);

  std::shared_ptr<Model> GetModel();

  bool HasModel() const;

private:
  std::shared_ptr<Entity> parent_;

  std::shared_ptr<Model> model_;

  glm::vec3 location_;
  glm::quat rotation_;
  glm::vec3 scale_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_ENTITY_H_