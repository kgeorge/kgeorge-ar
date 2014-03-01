#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <iostream>
#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>
#include <math.h>

#include "oglBackground.h"
#include "shaderSupport.h"
#include "util.h"

using namespace std;
using namespace cv;

OGLBackground::OGLBackground():
bTextureInitialized(false),
backgroundTextureId(0),
backgroundTextureVerticesVBO(0),
backgroundTextureIndicesVBO(0),
backgroundShaderProgramId(0),
backgroundTexUniformLocationInShaderProgram(0){
}


OGLBackground::~OGLBackground() {
}

void OGLBackground::processFrame(Mat &frame) {
    frame.copyTo(backgroundImage);
}

void OGLBackground::initGeometry(int w, int h) {
    assert(!bTextureInitialized);
    
    backgroundTextureVerts[0].location[0] = 0.0f;
    backgroundTextureVerts[0].location[1] = 0.0f;
    
    backgroundTextureVerts[1].location[0] = w;
    backgroundTextureVerts[1].location[1] = 0.0f;
    
    backgroundTextureVerts[2].location[0] = 0.0f;
    backgroundTextureVerts[2].location[1] = h;
    
    backgroundTextureVerts[3].location[0] = w;
    backgroundTextureVerts[3].location[1] = h;
    
    backgroundTextureVerts[0].textureCoords[0] = 0.0f;
    backgroundTextureVerts[0].textureCoords[1] = 1.0f;
    
    backgroundTextureVerts[1].textureCoords[0] = 1.0f;
    backgroundTextureVerts[1].textureCoords[1] = 1.0f;
    
    backgroundTextureVerts[2].textureCoords[0] = 0.0f;
    backgroundTextureVerts[2].textureCoords[1] = 0.0f;
    
    backgroundTextureVerts[3].textureCoords[0] = 1.0f;
    backgroundTextureVerts[3].textureCoords[1] = 0.0f;
    
    backgroundTextureIndices[0] = 0;
    backgroundTextureIndices[1] = 1;
    backgroundTextureIndices[2] = 2;
    
    backgroundTextureIndices[3] = 1;
    backgroundTextureIndices[4] = 3;
    backgroundTextureIndices[5] = 2;
}

void OGLBackground::initGLObjects() {
    assert(!bTextureInitialized);
    
    glGenTextures(1, &backgroundTextureId);
    glBindTexture(GL_TEXTURE_2D, backgroundTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glGenBuffers(1, &backgroundTextureVerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, backgroundTextureVerticesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BackgroundVertex)*4, (const void *)(&backgroundTextureVerts[0].location[0]), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &backgroundTextureIndicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundTextureIndicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*6, (const void *) &backgroundTextureIndices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    _setupShaders();
}



void OGLBackground::cleanupGLObjects() {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &backgroundTextureId);
    bTextureInitialized = false;
    backgroundTextureId = 0;
    
    glDeleteBuffers(1, &backgroundTextureIndicesVBO);
    glDeleteBuffers(1, &backgroundTextureVerticesVBO);
}



void OGLBackground::draw() {
    
    const float w = backgroundImage.cols;
    const float h = backgroundImage.rows;
    
    if (!bTextureInitialized){
        initGeometry(w, h);
        initGLObjects();
        bTextureInitialized = true;
    }
    glUseProgram(backgroundShaderProgramId);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backgroundTextureId);
    switch(backgroundImage.channels()) {
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, backgroundImage.data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, backgroundImage.data);
            break;
            
        default:
            throw runtime_error("unsupported image format");
            
    }
    
    glUniform1i(backgroundShaderProgramId, 0);
    const GLfloat proj[]              = { 2.0f/w, 0, 0, 0, 0, 2.0f/h, 0, 0, 0, 0, 1, 0, -1, -1, 0, 1 };
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundTextureId);
    glBindBuffer(GL_ARRAY_BUFFER, backgroundTextureVerticesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundTextureIndicesVBO);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    
    glVertexPointer(2, //2 components per location
                    GL_FLOAT, //type
                    sizeof(BackgroundVertex),
                    (const void*)(0)
                    );
    
    glTexCoordPointer(2, //2 components per tex coords
                      GL_FLOAT, //type
                      sizeof(BackgroundVertex), //stride
                      (const void *)(8)//offset to tex coords
                      );
    
    glColor4f(1,1,1,1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (char*) NULL+0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
    
}

void OGLBackground::cleanup() {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &backgroundTextureId);
    bTextureInitialized = false;
    backgroundTextureId = 0;
    cleanupGLObjects();
}

void OGLBackground::_setupShaders() {
    backgroundShaderProgramId = ShaderSupport::makeShader( "./shaders/simple.vs",  "./shaders/simple.fs" );
    
    backgroundTexUniformLocationInShaderProgram = glGetUniformLocation(backgroundShaderProgramId, "backgroundTex");
    validate(backgroundTexUniformLocationInShaderProgram >= 0, string("cant find uniform location: ") + string("backgroundTexUniformLocationInShaderProgram"));
}
