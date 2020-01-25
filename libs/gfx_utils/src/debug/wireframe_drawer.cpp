#include "gfx_utils/debug/wireframe_drawer.h"

#include <string>
#include <iostream>
#include <cassert>

#include "gfx_utils/mesh.h"

namespace gfx_utils {

static const std::string kWireframeVertShader = "shaders/wireframe.vert";
static const std::string kWireframeFragShader = "shaders/wireframe.frag";

bool WireframeDrawer::Initialize() {
  if (!wireframe_program_.CreateProgram(kWireframeVertShader, 
                                        kWireframeFragShader)) {
    std::cerr << "Could not create wireframe program" << std::endl;
    return false;                                      
  }

  glGenVertexArrays(1, &wireframe_vao_);

  is_initialized_ = true;

  return true;
}

void WireframeDrawer::Destroy() {
  assert(is_initialized_);

  glDeleteVertexArrays(1, &wireframe_vao_);

  wireframe_program_.DestroyProgram();
}

void WireframeDrawer::Draw(const Mesh* mesh, GLuint pos_vbo_id, GLuint ibo_id, 
                           const glm::mat4& mvp_mat) {
  glUseProgram(wireframe_program_.GetProgramId());

  wireframe_program_.GetUniform("mvp_mat").Set(mvp_mat);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glBindVertexArray(wireframe_vao_);

  glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
  // TODO(colintan): This may be wrong - consider writing a wireframe shader
  // using barycentric coordinates
  glDrawElements(GL_LINE_STRIP, mesh->num_verts, GL_UNSIGNED_INT,
      (void*)0);

  glBindVertexArray(0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glUseProgram(0);
}

} // namespace gfx_utils