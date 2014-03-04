
#if !defined(SHADER_SUPPORT_H_)
#define SHADER_SUPPORT_H_
#include <string>

#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>
#include "kgUtil.h"

namespace ShaderSupport {

    namespace Detail {
        void _linkShader(GLuint programId);
        GLuint _createShader(const std::string &filename, GLenum shaderType);
        GLuint _createShaderCore(GLenum shaderType, const char **pzShaderText, int shader_count, int shaderSourceLen);
    };

    GLuint makeShaderProgram(const std::string &filename_vs, const std::string &filename_fs);

    template<int N, int M>
    GLuint makeShaderProgramFromText(const char *(&vs_text)[N], const char *(&ps_text)[M]) {

        GLuint programId = glCreateProgram();

        validate(programId != 0, "error creating shader program");

        GLuint vs = Detail::_createShaderCore(GL_VERTEX_SHADER, vs_text, N,  0);
        GLuint fs = Detail::_createShaderCore(GL_FRAGMENT_SHADER, ps_text, M, 0);

        glAttachShader(programId, vs);
        glAttachShader(programId, fs);

        Detail::_linkShader(programId);

        return programId;
    }
    void cleanupShaderProgram(GLuint programId);
    void validateShader(GLuint programId);
};


#endif //SHADER_SUPPORT_H_
