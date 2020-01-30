add_executable(shadow_cube src/main.cpp src/app.cpp)

# Use C++11
target_compile_features(shadow_cube PUBLIC cxx_std_11)
set_target_properties(shadow_cube PROPERTIES CXX_EXTENSIONS OFF)

# Use OpenGL
target_include_directories(shadow_cube PUBLIC "${OPENGL_INCLUDE_DIRS}")
target_link_libraries(shadow_cube PUBLIC OpenGL::GL)
target_link_libraries(shadow_cube PUBLIC OpenGL::GLU)

# Use GLEW
target_include_directories(shadow_cube PUBLIC "${GLEW_INCLUDE_DIRS}")
target_link_libraries(shadow_cube PUBLIC 
    "${GLEW_STATIC_LIBRARIES}/glew32.lib")
# TODO(colintan): Find a more graceful way to do this - maybe create a function
# that does what's needed to get GLEW working
add_custom_command(TARGET shadow_cube POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy "${GLEW_SHARED_LIBRARIES}/glew32.dll" 
    "${CMAKE_CURRENT_BINARY_DIR}")

# Use GLFW
target_include_directories(shadow_cube PUBLIC "${GLFW_INCLUDE_DIRS}")
target_link_libraries(shadow_cube PUBLIC 
    "${GLFW_STATIC_LIBRARIES}/glfw3.lib")

# Use our gfx_utils library
target_link_libraries(shadow_cube PUBLIC gfx_utils)

# In the binary folder, create a symlink to the shaders folder
add_custom_command(TARGET shadow_cube POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/shaders" 
    "${CMAKE_CURRENT_BINARY_DIR}/shaders")

# In the binary folder, create a symlink to the assets folder
add_custom_command(TARGET shadow_cube POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/assets" 
    "${CMAKE_CURRENT_BINARY_DIR}/assets")