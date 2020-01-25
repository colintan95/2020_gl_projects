add_library(gl_utils src/program.cpp)

target_include_directories(gl_utils PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Use glm
target_include_directories(gl_utils PUBLIC "${GLM_INCLUDE_DIRS}")

# Use OpenGL
target_include_directories(gl_utils PUBLIC "${OPENGL_INCLUDE_DIRS}")
target_link_libraries(gl_utils PUBLIC OpenGL::GL)
target_link_libraries(gl_utils PUBLIC OpenGL::GLU)

# Use GLEW
target_include_directories(gl_utils PUBLIC "${GLEW_INCLUDE_DIRS}")
target_link_libraries(gl_utils PUBLIC 
    "${GLEW_STATIC_LIBRARIES}/glew32.lib")
# TODO(colintan): Find a more graceful way to do this - maybe create a function
# that does what's needed to get GLEW working
add_custom_command(TARGET gl_utils POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy "${GLEW_SHARED_LIBRARIES}/glew32.dll" 
    "${CMAKE_CURRENT_BINARY_DIR}")