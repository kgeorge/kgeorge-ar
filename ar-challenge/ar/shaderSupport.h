
#if !defined(SHADER_SUPPORT_H_)
#define SHADER_SUPPORT_H_
#include <string>

#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>

namespace ShaderSupport {
    GLuint makeShader(const std::string &filename_vs, const std::string &filename_fs);
};


#endif //SHADER_SUPPORT_H_
