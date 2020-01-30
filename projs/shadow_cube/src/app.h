#ifndef APP_H_
#define APP_H_

#include <vector>
#include <memory>
#include <string>

#include <GL/glew.h>
#if defined(WIN32)
#include <GL/gl.h>
#endif

#include "gfx_utils/window/window.h"
#include "gfx_utils/window/camera.h"
#include "gfx_utils/program.h"
#include "gfx_utils/scene/scene.h"
#include "gfx_utils/lights.h"
#include "gfx_utils/mesh.h"
#include "gfx_utils/gl/gl_resource_manager.h"

class App {
public:
  void Run();

private:
  void MainLoop();

  void ShadowPass();
  
  void LightPass();

  void LightPass_SetTransformUniforms_Mesh(gfx_utils::Mesh& mesh,
                                           glm::mat4& model_mat,
                                           glm::mat4& view_mat,
                                           glm::mat4& proj_mat);
  void LightPass_SetMaterialUniforms_Mesh(gfx_utils::Mesh& mesh);
  void LightPass_SetLightUniforms_Mesh(gfx_utils::Mesh& mesh,
                                       glm::mat4& model_mat,
                                       glm::mat4& view_mat);

  void Startup();

  void Cleanup();

private:
  gfx_utils::Window window_;
  gfx_utils::Camera camera_;

  gfx_utils::Program light_pass_program_;
  gfx_utils::Program shadow_pass_program_;

  gfx_utils::Scene scene_;

  gfx_utils::GLResourceManager resource_manager_;

  std::vector<std::shared_ptr<gfx_utils::Spotlight>> spotlights_;

  GLuint light_pass_vao_id_;

  GLuint shadow_pass_vao_id_;
  std::vector<GLuint> shadow_tex_id_list_;
  std::vector<GLuint> shadow_fbo_id_list_;
};

#endif // APP_H_