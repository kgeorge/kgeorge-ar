
#include <iostream>
#include <fstream>
#include <strstream>
#include <exception>

#include "kgShaderSupport.h"
#include "kgUtil.h"

using namespace std;


namespace {

    void _loadFile(const string & filename, string &shaderSource) {
        ifstream shaderFile(filename);
        string line;
        if(!shaderFile.is_open()) {
            throw runtime_error(string("cant find file!") + filename);
        }
        while( shaderFile.good() ) {
            line.clear();
            getline( shaderFile, line );
            shaderSource += line + "\n";
        }
        shaderFile.close();
    }

    bool _getShaderInfo(GLuint s, string &log) {
        int infologlength;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH,&infologlength);
        if(infologlength > 0) {
            int charsRead;
            char * info = new char[infologlength];
            glGetShaderInfoLog(s, infologlength, &charsRead, info);
            log.assign(info);
            delete [] info;
            return true;
        }
        return false;
    }




}
namespace ShaderSupport {

   namespace Detail {

        GLuint _createShaderCore(GLenum shaderType, const char **pzShaderText, int shader_count, int shaderSourceLen) {
            GLint s = glCreateShader( shaderType );
            int *shaderSourceLenAddr = &shaderSourceLen;
            if(shaderSourceLen <= 0) {
                shaderSourceLenAddr = NULL;
            }
            glShaderSource(s, shader_count, const_cast<const char**> (pzShaderText), shaderSourceLenAddr);
            GLint shaderCompiled;
            glCompileShader(s);
            glGetShaderiv(s, GL_COMPILE_STATUS, &shaderCompiled);
            string log;
            _getShaderInfo(s,log);
            validate(shaderCompiled != GL_FALSE, string("shader compilation failed: ") + log);
            if(!log.empty()) {
                cerr << "Warning:" << log.c_str();
            }
            return s;
        }



        GLuint _createShader(const string &filename, GLenum shaderType) {
            string shaderSource;
            _loadFile(filename,  shaderSource);
            const char *shaderSourceChar = shaderSource.c_str();
            int shaderSourceLen = static_cast<int>(shaderSource.length());
            return _createShaderCore(shaderType, &shaderSourceChar, 1, shaderSourceLen);
        }

        void _linkShader(GLuint programId) {
            GLint shaderLinked;
            glLinkProgram(programId);
            glGetProgramiv(programId, GL_LINK_STATUS, &shaderLinked);

            string log;
            _getShaderInfo(programId,log);
            validate(shaderLinked!= GL_FALSE, string("shader linking failed: ") + log);

            if(!log.empty()) {
                cerr << "Warning:" << log.c_str();
            }
            return;
        }

   }
    void validateShader(GLuint programId) {
        glValidateProgram(programId);
        GLint shaderValidated;
        glGetProgramiv(programId, GL_VALIDATE_STATUS, &shaderValidated);

        string log;
        _getShaderInfo(programId,log);
        validate(shaderValidated!= GL_FALSE, string("shader validated failed: ") + log);

        if(!log.empty()) {
            cerr << "Warning:" << log.c_str();
        }
        return;
    }



    GLuint makeShaderProgram(const string &filepath_vs, const std::string &filepath_fs) {

        GLuint programId = glCreateProgram();

        validate(programId != 0, "error creating shader program");


        GLuint vs = Detail::_createShader(filepath_vs, GL_VERTEX_SHADER);
        GLuint fs = Detail::_createShader(filepath_fs, GL_FRAGMENT_SHADER);

        glAttachShader(programId, vs);
        glAttachShader(programId, fs);

        Detail::_linkShader(programId);

        return programId;
    }

    void cleanupShaderProgram(GLuint programId) {
        const GLsizei kMaxNumShadersAttachedToAProgram=10;
        GLuint shadersAttached[kMaxNumShadersAttachedToAProgram];
        GLsizei numShadersAttachedToThisProgram;
        glGetAttachedShaders(programId,
                             kMaxNumShadersAttachedToAProgram,
                             &numShadersAttachedToThisProgram,
                             shadersAttached
                             );
        for(GLsizei i=0; i < numShadersAttachedToThisProgram; i++) {
            glDetachShader(programId, shadersAttached[i]);
        }
        for(GLsizei i=0; i < numShadersAttachedToThisProgram; i++) {
            glDeleteShader(shadersAttached[i]);
        }
        glDeleteProgram(programId);
    }

}