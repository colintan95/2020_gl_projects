add_library(gfx_utils src/mesh.cpp src/primitives.cpp src/entity.cpp
                      src/primitives.cpp src/program.cpp src/texture.cpp 
                      src/window/window.cpp src/window/camera.cpp
                      src/debug/wireframe_drawer.cpp
                      src/resources/resource_manager.cpp)

target_include_directories(gfx_utils PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Use tinyobjloader
target_include_directories(gfx_utils PUBLIC "${TINYOBJLOADER_INCLUDE_DIRS}")

# Use glm
target_include_directories(gfx_utils PUBLIC "${GLM_INCLUDE_DIRS}")

# Use stb
target_include_directories(gfx_utils PUBLIC "${STB_INCLUDE_DIRS}")

# Use nlohmann json
target_include_directories(gfx_utils PUBLIC "${JSON_INCLUDE_DIRS}")

# Use OpenGL
target_include_directories(gfx_utils PUBLIC "${OPENGL_INCLUDE_DIRS}")
target_link_libraries(gfx_utils PUBLIC OpenGL::GL)
target_link_libraries(gfx_utils PUBLIC OpenGL::GLU)

# Use GLEW
target_include_directories(gfx_utils PUBLIC "${GLEW_INCLUDE_DIRS}")
target_link_libraries(gfx_utils PUBLIC 
    "${GLEW_STATIC_LIBRARIES}/glew32.lib")
# TODO(colintan): Find a more graceful way to do this - maybe create a function
# that does what's needed to get GLEW working
add_custom_command(TARGET gfx_utils POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy "${GLEW_SHARED_LIBRARIES}/glew32.dll" 
    "${CMAKE_CURRENT_BINARY_DIR}")

# Use GLFW
target_include_directories(gfx_utils PUBLIC "${GLFW_INCLUDE_DIRS}")
target_link_libraries(gfx_utils PUBLIC 
    "${GLFW_STATIC_LIBRARIES}/glfw3.lib")