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

  void GeometryPass();
  void SSAOPass();
  void LightPass();

  void Startup();

  void SetupGeometryPass();
  void SetupSSAOPass();
  void SetupLightPass();

  void Cleanup();

private:
  gfx_utils::Window window_;
  gfx_utils::Camera camera_;

  gfx_utils::Scene scene_;

  gfx_utils::GLResourceManager resource_manager_;

  std::vector<std::shared_ptr<gfx_utils::PointLight>> lights_;

  gfx_utils::Program geom_pass_program_;
  GLuint geom_pass_vao_;

  GLuint gbuf_fbo_;
  GLuint gbuf_depth_rbo_;

  GLuint gbuf_pos_tex_;
  GLuint gbuf_normal_tex_;
  GLuint gbuf_ambient_tex_;

  gfx_utils::Program ssao_pass_program_;
  gfx_utils::Program ssao_blur_program_;

  GLuint ssao_fbo_;
  GLuint ssao_color_tex_;

  GLuint ssao_blur_fbo_;
  GLuint ssao_blur_tex_;

  GLuint ssao_pass_vao_;
  GLuint ssao_pass_quad_vbo_;

  std::vector<glm::vec3> ssao_kernel_;

  GLuint ssao_noise_tex_;

  gfx_utils::Program light_pass_program_;

  GLuint light_pass_vao_;
  GLuint light_pass_quad_vbo_;
};

#endif // APP_H_