#include "gfx_utils/gl/gl_resource_manager.h"

namespace gfx_utils {

void GLResourceManager::CreateGLResources() {
  const auto& models = scene_->GetModels();

  for (auto model_ptr : models) {
    for (gfx_utils::Mesh& mesh : model_ptr->GetMeshes()) {
      CreateMeshResources(mesh);
    }
  }

  // Maps the texture name to the corresponding Texture pointer
  auto& texture_name_map = scene_->GetTextureNameMap();

  // Allocate textures 
  for (auto it = texture_name_map.begin(); it != texture_name_map.end(); ++it) {
    auto texture_ptr = it->second;

    CreateTextureResources(*texture_ptr);
  }
}

void GLResourceManager::Cleanup() {
  for (auto it : texture_gl_id_map_) {
    glDeleteTextures(1, &it.second);
  }

  for (auto it : mesh_pos_gl_id_map_) {
    glDeleteBuffers(1, &it.second);
  }

  for (auto it : mesh_normal_gl_id_map_) {
    glDeleteBuffers(1, &it.second);
  }

  for (auto it : mesh_texcoord_gl_id_map_) {
    glDeleteBuffers(1, &it.second);
  }

  for (auto it : mesh_mtl_id_gl_id_map_) {
    glDeleteBuffers(1, &it.second);
  }

  for (auto it : mesh_ibo_gl_id_map_) {
    glDeleteBuffers(1, &it.second);
  }
}

void GLResourceManager::CreateMeshResources(const Mesh& mesh) {
  MeshId id = mesh.id;
      
  GLuint pos_vbo_id;
  glGenBuffers(1, &pos_vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_id);
  glBufferData(GL_ARRAY_BUFFER, mesh.pos_data.size() * 3 * sizeof(float),
                &mesh.pos_data[0], GL_STATIC_DRAW);
  mesh_pos_gl_id_map_[id] = pos_vbo_id;

  GLuint normal_vbo_id;
  glGenBuffers(1, &normal_vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_id);
  glBufferData(GL_ARRAY_BUFFER, mesh.normal_data.size() * 3 * sizeof(float),
                &mesh.normal_data[0], GL_STATIC_DRAW);
  mesh_normal_gl_id_map_[id] = normal_vbo_id;

  GLuint texcoord_vbo_id;
  glGenBuffers(1, &texcoord_vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_id);
  glBufferData(GL_ARRAY_BUFFER, mesh.texcoord_data.size() * 2 * sizeof(float),
                &mesh.texcoord_data[0], GL_STATIC_DRAW);
  mesh_texcoord_gl_id_map_[id] = texcoord_vbo_id;

  if (mesh.material_list.size() != 0) {
    GLuint mtl_vbo_id;
    glGenBuffers(1, &mtl_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, mtl_vbo_id);
    glBufferData(GL_ARRAY_BUFFER,
                  mesh.mtl_id_data.size() * sizeof(unsigned int),
                  &mesh.mtl_id_data[0], GL_STATIC_DRAW);
    mesh_mtl_id_gl_id_map_[id] = mtl_vbo_id;
  }
  else {
    mesh_mtl_id_gl_id_map_[id] = 0;
  }

  GLuint ibo_id;
  glGenBuffers(1, &ibo_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               mesh.index_data.size() * sizeof(uint32_t),
               &mesh.index_data[0], GL_STATIC_DRAW);
  mesh_ibo_gl_id_map_[id] = ibo_id;
}

void GLResourceManager::CreateTextureResources(const Texture& texture) {
  GLuint texture_id;
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  GLenum format = texture.has_alpha ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, format, texture.tex_width,
               texture.tex_height, 0, format, GL_UNSIGNED_BYTE,
               &texture.tex_data[0]);

  glBindTexture(GL_TEXTURE_2D, 0);

  texture_gl_id_map_[texture.id] = texture_id;
}

GLuint GLResourceManager::GetMeshVboId(MeshId id, VertType vert_type) {
  switch(vert_type) {
  case kVertTypePosition:
    return mesh_pos_gl_id_map_[id];
    break;
  case kVertTypeNormal:
    return mesh_normal_gl_id_map_[id];
    break;
  case kVertTypeTexcoord:
    return mesh_texcoord_gl_id_map_[id];
    break;
  case kVertTypeMtlId:
    return mesh_mtl_id_gl_id_map_[id];
    break;
  default:
    return 0;
  }
}

GLuint GLResourceManager::GetMeshIboId(MeshId id) {
  return mesh_ibo_gl_id_map_[id];
}

GLuint GLResourceManager::GetTextureId(TextureId id) {
  return texture_gl_id_map_[id];
}

GLuint GLResourceManager::GetTextureId(const std::string& texname) {
  auto texture_ptr = scene_->GetTexture(texname);

  return texture_gl_id_map_[texture_ptr->id];
}

} // namespace gfx_utils