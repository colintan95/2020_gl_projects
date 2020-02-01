#include "gfx_utils/mesh.h"

#include "tinyobjloader/tiny_obj_loader.h"

#include <iostream>
#include <tuple>
#include <unordered_map>

#include "gfx_utils/texture.h"

namespace gfx_utils {

static MeshId mesh_id_counter = 0;

Mesh::Mesh() {
  ++mesh_id_counter;

  id = mesh_id_counter;
}

void ClearMesh(Mesh *mesh) {
  mesh->pos_data.clear();
  mesh->normal_data.clear();
  mesh->texcoord_data.clear();

  mesh->index_data.clear();

  mesh->mtl_id_data.clear();

  mesh->num_verts = 0;

  mesh->material_list;
}

}