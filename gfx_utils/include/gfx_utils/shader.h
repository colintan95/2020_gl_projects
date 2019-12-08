#ifndef GFX_UTILS_SHADER_H_
#define GFX_UTILS_SHADER_H_

#include <GL/glew.h>
#if defined(WIN32)
#include <GL/gl.h>
#endif

#include <string>

namespace gfx_utils {

bool CreateProgram(GLuint *out_program_id,
                   const std::string& vert_shader_path,
                   const std::string& frag_shader_path);

bool LoadShaderSource(std::string* out_str, const std::string& path);

}

#endif