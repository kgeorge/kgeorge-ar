#include <iostream>
#include <fstream>
#include <strstream>
#include <exception>

#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>
#include "kgUtil.h"

using namespace std;
using namespace cv;

void validate(bool bval, const std::string& msg) {
    if(!bval) {
        throw runtime_error(msg.c_str());
    }
}

void check_gl_error(const char *file, int line) {
        GLenum err (glGetError());

        while(err!=GL_NO_ERROR) {
                string error;

                switch(err) {
                        case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
                        case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
                        case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
                        case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
                        case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
                }

                cerr << "GL_" << error.c_str() <<" - "<<file<<":"<<line<<endl;
                //throw runtime_error("glerror");
                err=glGetError();
        }
}


double angle(const Point &p0, const Point &p1, const Point &p2 ) {
    double dx0 = p0.x - p1.x;
    double dx2 = p2.x - p1.x;
    double dy0 = p0.y - p1.y;
    double dy2 = p2.y - p1.y;
    double num = dx0 * dx2 + dy0 * dy2;
    double denom = sqrt((dx0 * dx0 + dy0  * dy0) * (dx2 * dx2 + dy2 * dy2) + 1.0e-10);
    return num/denom;
}
