add_library(graphics_utils src/shader.cpp)

target_include_directories(graphics_utils PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include")