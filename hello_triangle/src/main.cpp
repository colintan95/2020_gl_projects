#include <iostream>
#include <cstdlib>

#include "GL/glew.h"

#if defined(WIN32)
#include "GL/gl.h"
#endif

#include "GLFW/glfw3.h"

static const GLfloat vert_data[] = {
  -0.5f, -0.5f, 0.0f,
   0.5f, -0.5f, 0.0f,
   0.0f,  0.5f, 0.0f
};

int main(int argc, char *argv[]) {

  // TODO(colintan): Check if we need glewExperimental to be true
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    exit(1);
  }

  std::cout << "GLFW initialized" << std::endl;

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // TODO(colintan): Check if we really need this
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow *window;
  window = glfwCreateWindow(1024, 768, "Hello Triangle", nullptr, nullptr);

  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    exit(1);
  }

  glfwMakeContextCurrent(window);

  // TODO(colintan): Is this needed?
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    exit(1);
  }

  std::cout << "GLEW initialized" << std::endl;

  GLuint vao_id;
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GLuint pos_vbo_id;
  glGenBuffers(1, &pos_vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vert_data), vert_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);

  bool should_quit = false;

  while (!should_quit) {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao_id);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
    glVertexAttribPointer(
        0,          // index
        3,          // size
        GL_FLOAT,   // type
        GL_FALSE,   // normalized
        0,          // stried
        (void*) 0   // pointer
    );

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();

    if (glfwWindowShouldClose(window) == 1) {
      should_quit = true;
    }
  }

  glDeleteBuffers(1, &pos_vbo_id);
  glDeleteVertexArrays(1, &vao_id);

  glfwTerminate();

  return 0;
}