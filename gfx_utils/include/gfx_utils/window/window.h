#ifndef GFX_UTILS_WINDOW_WINDOW_
#define GFX_UTILS_WINDOW_WINDOW_

#include "GL/glew.h"
#if defined(WIN32)
#include "GL/gl.h"
#endif
#include "GLFW/glfw3.h"

#include <glm/matrix.hpp>

#include <functional>
#include <unordered_map>

namespace gfx_utils {

enum KeyEventType {
  KEY_EVENT_DOWN,
  KEY_EVENT_UP
};

enum KeyActionType {
  KEY_ACTION_PRESS,
  KEY_ACTION_HOLD
};

// There should only be a single Window class in the application
class Window {

public:
  Window();
  ~Window();

  bool Inititalize();

  void SwapBuffers();

  void TickMainLoop();

  bool ShouldQuit();

  void RegisterKeyBinding(int key, KeyActionType action, 
                          std::function<void()> func);

private:
  struct KeyActionInfo {
    bool status = false;
    KeyActionType type ; // Press or Hold
    std::function<void()> func;
  };

private:
  void TriggerKeyActions();

  void HandleKeyEvent(int key, KeyEventType event);

  // GLFW input callbacks - can only be static since GLFW can't register
  // a member function as a callback
  static void KeyboardInputCallback(GLFWwindow *window, int key, int scancode, 
                                    int action, int mods);
  static void MouseInputCallback(GLFWwindow *window, double xpos, double ypos);

private:
  GLFWwindow *glfw_window_;

  bool is_initialized_;

  std::unordered_map<int, KeyActionInfo> key_action_map_;

  // Used to ensure that there's only a single instance of the class
  static bool has_instance_;

  // Needed so that GLFW callbacks can access the window
  static Window *instance_ptr_;
};

} // namespace gfx_utils

#endif