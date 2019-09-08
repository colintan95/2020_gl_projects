add_library(gfx_utils src/shader.cpp src/mesh.cpp src/texture.cpp)

target_include_directories(gfx_utils PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Use tinyobjloader
target_include_directories(gfx_utils PUBLIC "${TINYOBJLOADER_INCLUDE_DIRS}")

# Use glm
target_include_directories(gfx_utils PUBLIC "${GLM_INCLUDE_DIRS}")

# Use stb
target_include_directories(gfx_utils PUBLIC "${STB_INCLUDE_DIRS}")