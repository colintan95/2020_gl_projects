#include "gfx_utils/renderers/simple_renderer.h"

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
  "  vec3 ambient  = materials[frag_mtl_id].ambient_color;\n"
  "  vec3 diffuse  = materials[frag_mtl_id].diffuse_color;\n"
  "  vec3 emission = materials[frag_mtl_id].emission_color;\n"
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

  if (!program_.CreateProgram(vert_shader_src, frag_shader_src)) {
    return false;
  }

  glUseProgram(program_.GetProgramId());

  glGenVertexArrays(1, &vao_id_);

  

  return true;
}

void SimpleRenderer::Destroy() {

}

void SimpleRenderer::Render(const EntityList& entities) {

}

} // namespace gfx_utils