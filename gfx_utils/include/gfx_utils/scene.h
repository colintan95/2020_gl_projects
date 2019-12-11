#ifndef GFX_UTILS_SCENE_H_
#define GFX_UTILS_SCENE_H_

#include <vector>

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

namespace gfx_utils {

// Foward declaration
struct Mesh;

class SceneObject {

public:
  SceneObject(glm::vec3 location, glm::vec3 scale, float yaw, float pitch, 
              float roll);
  SceneObject();

  glm::mat4 CalcTransform() const;

  void AddMesh(Mesh* mesh);
  void SetParent(SceneObject* parent);

  void SetScale(glm::vec3 scale) { scale_ = scale; }

  const std::vector<Mesh*>& GetMeshes() const { return meshes_; }

  bool HasMeshes() const { return meshes_.size() != 0; }

private:
  SceneObject* parent_;

  std::vector<Mesh*> meshes_;

  glm::vec3 location_;
  glm::quat rotation_;
  glm::vec3 scale_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_SCENE_H_