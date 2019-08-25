cmake_minimum_required(VERSION 3.1)
project(gl_projects)

# Find OpenGL
find_package(OpenGL)
if(OPENGL_FOUND)
  message(STATUS "Found OpenGL")
else()
  message(FATAL_ERROR "Unable to find OpenGL")
endif()

# Use GLEW to work with OpenGL
find_package(GLEW)
if(GLEW_FOUND)
  message(STATUS "Found GLEW")
else()
  # TODO(colintan): Get rid of hardcoding
  # HACK(colintan): Hardcode paths to GLEW for now
  if(TRUE)
    set(GLEW_INCLUDE_DIRS "D:/glew-2.1.0/include")
    set(GLEW_STATIC_LIBRARIES "D:/glew-2.1.0/lib/Release/x64")
    set(GLEW_SHARED_LIBRARIES "D:/glew-2.1.0/bin/Release/x64")
  else()
    message(FATAL_ERROR "Unable to find GLEW")
  endif()
endif()

# TODO(colintan): Get rid of hardcoding
# HACK(colintan): Hardcode paths to GLFW for now
set(GLFW_INCLUDE_DIRS "D:/glfw-3.3/include")
set(GLFW_STATIC_LIBRARIES "D:/glfw-3.3/lib-vc2019/")

# Library that contains utils commonly used in the projects
add_subdirectory(graphics_utils)

# Place the targets for the graphics projects here
add_subdirectory(hello_triangle)