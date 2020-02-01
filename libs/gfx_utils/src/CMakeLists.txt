target_sources(gfx_utils
  PRIVATE
    entity.cpp
    mesh.cpp
    primitives.cpp
    program.cpp
    texture.cpp
)

# add_subdirectory(debug)
add_subdirectory(gl)
add_subdirectory(renderers)
add_subdirectory(scene)
add_subdirectory(window)