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
#include "gfx_utils/resource/resource_manager.h"
#include "gfx_utils/lights.h"
#include "gfx_utils/mesh.h"
#include "gfx_utils/gl/gl_resource_manager.h"
#include "gfx_utils/renderers/simple_renderer.h"

class App {
public:
  void Run();

private:
  void MainLoop();

  void ShadowPass();

  void Startup();

  void Cleanup();

private:
  gfx_utils::Window window_;
  gfx_utils::Camera camera_;

  gfx_utils::ResourceManager resource_manager_;

  gfx_utils::GLResourceManager gl_resource_manager_;

  gfx_utils::SimpleRenderer renderer_;
  
};

#endif // APP_H_