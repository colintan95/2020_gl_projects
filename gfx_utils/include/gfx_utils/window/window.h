#ifndef GFX_UTILS_WINDOW_WINDOW_
#define GFX_UTILS_WINDOW_WINDOW_

#include "GL/glew.h"
#if defined(WIN32)
#include "GL/gl.h"
#endif
#include "GLFW/glfw3.h"

#include <glm/matrix.hpp>

namespace gfx_utils {

class Window {
public:
  Window();
  ~Window();

  bool Inititalize();

  void SwapBuffers();

  void TickMainLoop();

  bool ShouldQuit();

  glm::mat4 GetViewMatrix();

private:
  void HandleKeyboardInput();

private:
  GLFWwindow *glfw_window_;

  bool is_initialized_;

  glm::mat4 view_mat_;
};

}

#endif