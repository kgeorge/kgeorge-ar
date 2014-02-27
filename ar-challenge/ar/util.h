#if !defined(UTIL_H_)
#define UTIL_H_
#include <string>


#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

GLuint createShader(const std::string &filename, GLenum shaderType);

#endif //UTIL_H_
