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

#include "kgBackground.h"
#include "kgShaderSupport.h"
#include "kgUtil.h"

using namespace std;
using namespace cv;

static const char * background_vs_source[] = {
"#version 330 \n",
"layout(location = 0) in vec2 position;\n",
"layout(location = 1) in vec2 uv;\n",
"uniform mat4 projection;\n",
"uniform mat4 model;\n",
"uniform mat4 view;\n",
"out vec2 vTexCoord;\n",
"void main(void){\n",
"    gl_Position = projection * view * model  * vec4(position.xy, 0.0, 1.0);\n",
"    vTexCoord = uv;\n",
"}\n"
};


static const char * background_fs_source[] = {
"#version 330\n",
"uniform sampler2D tex;\n",
"in vec2 vTexCoord;\n",
"out vec4 fragColor;\n",
"void main(void) {\n",
"    vec4 col = texture(tex, vTexCoord);\n",
"    fragColor = col;\n",
"}\n"

};


OGLBackground::OGLBackground():
bTextureInitialized(false),

backgroundTextureId(0),

backgroundVertexArray(0),
backgroundTextureVerticesVBO(0),
backgroundTextureIndicesVBO(0),

backgroundShaderProgramId(0),

backgroundTexUniformLocation(0),
backgroundPositionAttributeLocation(0),
backgroundUVAttributeLocation(0),
backgroundModelUniformLocation(0),
backgroundViewUniformLocation(0),
backgroundProjectionUniformLocation(0){
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
    
    glGenVertexArrays(1, &backgroundVertexArray);
    glBindVertexArray(backgroundVertexArray);
    
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
    glDeleteVertexArrays(1, &backgroundVertexArray);
    _cleanupShaders();
}



void OGLBackground::draw() {
    
    const float w = backgroundImage.cols;
    const float h = backgroundImage.rows;
    
    if (!bTextureInitialized){
        initGeometry(w, h);
        initGLObjects();
        bTextureInitialized = true;
    }
    //use the background shader program
    glUseProgram(backgroundShaderProgramId);
    
    //texture mapping
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glEnable(GL_TEXTURE_2D);
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
    
    //get uniform and attribute locations
    _getUniformAndAttributeLocations();
    
    
    //update model, view, projection uniforms
    const GLfloat proj[]              = { 2.0f/w, 0, 0, 0, 0, 2.0f/h, 0, 0, 0, 0, 1, 0, -1, -1, 0, 1 };
    const GLfloat modelOrView[]              = { 1.0f, 0, 0, 0,    0, 1.0f, 0, 0,    0, 0, 1.0f, 0,   0, 0, 0, 1.0f };
    glUniformMatrix4fv(backgroundProjectionUniformLocation, 1, GL_FALSE, proj);
    glUniformMatrix4fv(backgroundModelUniformLocation, 1, GL_FALSE, modelOrView);
    glUniformMatrix4fv(backgroundViewUniformLocation, 1, GL_FALSE, modelOrView);

    //vetex buffer object activation
    glBindBuffer(GL_ARRAY_BUFFER, backgroundTextureVerticesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundTextureIndicesVBO);
    glEnableVertexAttribArray(backgroundPositionAttributeLocation);
    glEnableVertexAttribArray(backgroundUVAttributeLocation);
    glVertexAttribPointer(
                    backgroundPositionAttributeLocation,
                    2, //2 components per location
                    GL_FLOAT, //type
                    GL_FALSE, //normalized
                    sizeof(BackgroundVertex),
                    (const void*)(0)
                    );
    glVertexAttribPointer(
                      backgroundUVAttributeLocation,
                      2, //2 components per tex coords
                      GL_FLOAT, //type
                      GL_TRUE,
                      sizeof(BackgroundVertex), //stride
                      (const void *)(8)//offset to tex coords
                      );
    
    //backgroundTexture in shader should point to texture unit0
    glUniform1i(backgroundTexUniformLocation, 0);
    
    //draw elements
    glColor4f(1,1,1,1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (char*) NULL+0);
    
    //disable, what was enabled
    glDisableVertexAttribArray(backgroundPositionAttributeLocation);
    glDisableVertexAttribArray(backgroundUVAttributeLocation);
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
    backgroundShaderProgramId = ShaderSupport::makeShaderProgramFromText( background_vs_source,  background_fs_source );
}

void OGLBackground::_getUniformAndAttributeLocations() {
    backgroundModelUniformLocation = glGetUniformLocation(backgroundShaderProgramId, "model");
    validate(backgroundModelUniformLocation >= 0, string("cant find uniform location: ") + string("model"));
    
    
    backgroundViewUniformLocation = glGetUniformLocation(backgroundShaderProgramId, "view");
    validate(backgroundViewUniformLocation >= 0, string("cant find uniform location: ") + string("view"));
    
    backgroundProjectionUniformLocation = glGetUniformLocation(backgroundShaderProgramId, "projection");
    validate(backgroundProjectionUniformLocation >= 0, string("cant find uniform location: ") + string("projection"));

    

    backgroundPositionAttributeLocation = glGetAttribLocation(backgroundShaderProgramId, "position");
    validate(backgroundPositionAttributeLocation >= 0, string("cant find attribute location: ") + string("position"));
    
    
    backgroundUVAttributeLocation = glGetAttribLocation(backgroundShaderProgramId, "uv");
    validate(backgroundUVAttributeLocation >= 0, string("cant find attribute location: ") + string("uv"));
    
    backgroundTexUniformLocation = glGetUniformLocation(backgroundShaderProgramId, "tex");
    validate(backgroundTexUniformLocation >= 0, string("cant find uniform location: ") + string("backgroundTexUniformLocation"));
    
}

void OGLBackground::_cleanupShaders() {
    ShaderSupport::cleanupShaderProgram( backgroundShaderProgramId );
}