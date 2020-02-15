#include "gfx_utils/renderers/simple_renderer.h"

#include <cassert>

#include "glm/glm.hpp"

namespace gfx_utils {

static const char vert_shader_src[] = 
  "#version 330 core\n"
  "\n"
  "layout(location = 0) in vec3 vert_pos;\n"
  "layout(location = 2) in vec2 vert_texcoord;\n"
  "layout(location = 3) in uint vert_mtl_id;\n"
  "\n"
  "out vec2 frag_texcoord;\n"
  "flat out uint frag_mtl_id;\n"
  "\n"
  "uniform mat4 mvp_mat;\n"
  "\n"
  "void main() {\n"
  "  gl_Position = mvp_mat * vec4(vert_pos, 1.0);\n"
  "\n"
  "  frag_texcoord = vert_texcoord;\n"
  "  frag_mtl_id = vert_mtl_id;\n"
  "}";

static const char frag_shader_src[] = 
  "#version 330 core\n"
  "\n"
  "in vec2 frag_texcoord;\n"
  "flat in uint frag_mtl_id;\n"
  "\n"
  "out vec4 out_color;\n"
  "\n"
  "struct Material {\n"
  "  vec3 ambient_color;\n"
  "  vec3 diffuse_color;\n"
  "  vec3 emission_color;\n"
  "  \n"
  "  bool has_ambient_tex;\n"
  "  bool has_diffuse_tex;\n"
  "  \n"
  "  sampler2D ambient_texture;\n"
  "  sampler2D diffuse_texture;\n"
  "};\n"
  "\n"
  "uniform Material materials[5];\n"
  "\n"
  "void main() {\n"
  "  vec3 ambient  = materials[frag_mtl_id].ambient_color * 0.5;\n"
  "  vec3 diffuse  = materials[frag_mtl_id].diffuse_color * 0.5;\n"
  "  vec3 emission = materials[frag_mtl_id].emission_color * 0.5;\n"
  "  \n"
  "  if (materials[frag_mtl_id].has_ambient_tex) {\n"
  "    ambient *= texture(materials[frag_mtl_id].ambient_texture,\n"
  "                       frag_texcoord).rgb;\n"
  "  }\n"
  "  if (materials[frag_mtl_id].has_diffuse_tex) {\n"
  "    diffuse *= texture(materials[frag_mtl_id].diffuse_texture,\n"
  "                       frag_texcoord).rgb;"
  "  }\n"
  "  \n"
  "  out_color = vec4(emission + ambient + diffuse, 1.0);\n"
  "}";

bool SimpleRenderer::Initialize() {
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  if (!program_.CreateFromSource(vert_shader_src, frag_shader_src)) {
    return false;
  }

  glUseProgram(program_.GetProgramId());

  glGenVertexArrays(1, &vao_id_);

  return true;
}

void SimpleRenderer::Destroy() {
  glDeleteVertexArrays(1, &vao_id_);

  // TODO(colintan): Destroy the program only when it was successfully created
}

void SimpleRenderer::Render(const EntityList& entities) {
  GLResourceManager* resource_manager = GetResourceManager();
  Window* window = GetWindow();
  Camera* camera = GetCamera();
  
  assert(resource_manager != nullptr);
  assert(window != nullptr);
  assert(camera != nullptr);

  glUseProgram(program_.GetProgramId());

  glViewport(0, 0, window->GetWindowWidth(), window->GetWindowHeight());

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 view_mat = camera->CalcViewMatrix();
  glm::mat4 proj_mat = glm::perspective(glm::radians(30.f),
                                        window->GetAspectRatio(),
                                        0.1f, 1000.f);

  for (auto entity_ptr : entities) {
    if (!entity_ptr->HasModel()) {
      continue;
    }

    for (auto& mesh : entity_ptr->GetModel()->GetMeshes()) {
      glm::mat4 model_mat = entity_ptr->ComputeTransform();

      SetTransformUniforms_Mesh(mesh, model_mat, view_mat, proj_mat);

      SetMaterialUniforms_Mesh(mesh);

      glBindVertexArray(vao_id_);

      GLuint pos_vbo_id = 
          resource_manager->GetMeshVboId(mesh.id, 
                                         gfx_utils::kVertTypePosition);
      glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      GLuint texcoord_vbo_id =
          resource_manager->GetMeshVboId(mesh.id, 
                                         gfx_utils::kVertTypeTexcoord);
      glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

      GLuint mtl_vbo_id =
          resource_manager->GetMeshVboId(mesh.id, 
                                         gfx_utils::kVertTypeMtlId);
      glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id);
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, 0, (GLvoid*)0);

      GLuint ibo_id = resource_manager->GetMeshIboId(mesh.id);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);

      glDrawElements(GL_TRIANGLES, mesh.num_verts, GL_UNSIGNED_INT, (void*)0);
    }
  }                                      
}


void SimpleRenderer::SetTransformUniforms_Mesh(gfx_utils::Mesh& mesh,
                                               glm::mat4& model_mat,
                                               glm::mat4& view_mat,
                                               glm::mat4& proj_mat) {
  glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;

  program_.GetUniform("mvp_mat").Set(mvp_mat);                                          
}

void SimpleRenderer::SetMaterialUniforms_Mesh(gfx_utils::Mesh& mesh) {
  GLResourceManager* resource_manager = GetResourceManager();

  const auto& mtl_list = mesh.material_list;
  for (int i = 0; i < mtl_list.size(); ++i) {
    const auto& mtl = mtl_list[i];

    program_.GetUniform("materials", i, "ambient_color")
            .Set(mtl.ambient_color);
    program_.GetUniform("materials", i, "diffuse_color")
            .Set(mtl.diffuse_color);
    program_.GetUniform("materials", i, "emission_color")
            .Set(mtl.emission_color);

    if (!mtl.ambient_texname.empty()) {
      program_.GetUniform("materials", i, "has_ambient_tex")
              .Set(true);

      glActiveTexture(GL_TEXTURE1);
      GLuint tex_gl_id = 
          resource_manager->GetTextureId(mtl.ambient_texname);
      glBindTexture(GL_TEXTURE_2D, tex_gl_id);
      program_.GetUniform("materials", i, "ambient_texture")
              .Set(1);
    }
    else {
      program_.GetUniform("materials", i, "has_ambient_tex")
              .Set(false);
    }

    if (!mtl.diffuse_texname.empty()) {
      program_.GetUniform("materials", i, "has_diffuse_tex")
              .Set(true);

      glActiveTexture(GL_TEXTURE2);
      GLuint tex_gl_id = 
          resource_manager->GetTextureId(mtl.diffuse_texname);
      glBindTexture(GL_TEXTURE_2D, tex_gl_id);
      program_.GetUniform("materials", i, "diffuse_texture")
              .Set(2);
    }
    else {
      program_.GetUniform("materials", i, "has_diffuse_tex")
              .Set(false);
    }
  }
}

} // namespace gfx_utils