#include <iostream>
#include <fstream>
#include <strstream>
#include <exception>
#include "util.h"

using namespace std;

static void _loadFile(const string & filename, string &shaderSource) {
    ifstream shaderFile(filename);
    strstream ss;
    string line;
    if(!shaderFile.is_open()) {
        throw runtime_error(string("cant find file!") + filename);
    }
    while( getline(shaderFile, line)) {
        ss << line;
    }
    shaderFile.close();
    shaderSource.assign(ss.str());
}


GLuint createShader(const string &filename, GLenum shaderType) {
    string shaderSource;
    _loadFile(filename,  shaderSource);
    const char *shaderSourceChar = shaderSource.c_str();
    GLint s = glCreateShader( shaderType );
    int shaderSourceLen = static_cast<int>(shaderSource.length());
    glShaderSource(s, 1, const_cast<const char**> (&shaderSourceChar), &shaderSourceLen);
    return s;
}