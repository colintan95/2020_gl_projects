#include "gfx_utils/scene/light_loader.h"

namespace gfx_utils {

std::shared_ptr<Light> LoadPointLight(const DataSource& data_src) {
  auto light = std::make_shared<PointLight>();
  
  light->position = data_src.GetEntry("position").val_vec3;
  light->diffuse_intensity = data_src.GetEntry("diffuse_intensity").val_vec3;
  light->specular_intensity = data_src.GetEntry("specular_intensity").val_vec3;
  light->camera_up = data_src.GetEntry("camera_up").val_vec3;

  return std::dynamic_pointer_cast<Light>(light);
}

std::shared_ptr<Light> LoadSpotlight(const DataSource& data_src) {
  auto light = std::make_shared<Spotlight>();
  
  light->position = data_src.GetEntry("position").val_vec3;
  light->diffuse_intensity = data_src.GetEntry("diffuse_intensity").val_vec3;
  light->specular_intensity = data_src.GetEntry("specular_intensity").val_vec3;
  light->direction = data_src.GetEntry("direction").val_vec3;
  light->cone_angle = data_src.GetEntry("cone_angle").val_float;
  light->camera_up = data_src.GetEntry("camera_up").val_vec3;

  return std::dynamic_pointer_cast<Light>(light);
}

} // namespace gfx_utils