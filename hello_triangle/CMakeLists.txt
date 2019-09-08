add_executable(hello_triangle src/main.cpp)

# Use C++11
target_compile_features(hello_triangle PUBLIC cxx_std_11)
set_target_properties(hello_triangle PROPERTIES CXX_EXTENSIONS OFF)

# Use OpenGL
target_include_directories(hello_triangle PUBLIC "${OPENGL_INCLUDE_DIRS}")
target_link_libraries(hello_triangle PUBLIC OpenGL::GL)
target_link_libraries(hello_triangle PUBLIC OpenGL::GLU)

# Use GLEW
target_include_directories(hello_triangle PUBLIC "${GLEW_INCLUDE_DIRS}")
target_link_libraries(hello_triangle PUBLIC 
    "${GLEW_STATIC_LIBRARIES}/glew32.lib")
# TODO(colintan): Find a more graceful way to do this - maybe create a function
# that does what's needed to get GLEW working
add_custom_command(TARGET hello_triangle POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy "${GLEW_SHARED_LIBRARIES}/glew32.dll" 
    "${CMAKE_CURRENT_BINARY_DIR}")

# Use GLFW
target_include_directories(hello_triangle PUBLIC "${GLFW_INCLUDE_DIRS}")
target_link_libraries(hello_triangle PUBLIC 
    "${GLFW_STATIC_LIBRARIES}/glfw3.lib")

# Use our gfx_utils library
target_link_libraries(hello_triangle PUBLIC gfx_utils)

# In the binary folder, create a symlink to the shaders folder
add_custom_command(TARGET hello_triangle POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/shaders" 
    "${CMAKE_CURRENT_BINARY_DIR}/shaders")