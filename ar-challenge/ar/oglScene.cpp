#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>

#include <iostream>
#include <math.h>
#include "oglScene.h"
#include "kgShaderSupport.h"

using namespace std;
using namespace cv;


static const char * geometry_vs_source[] = {
"#version 330  \n",
"layout(location = 0) in vec4 position;\n",
"layout(location = 1) in vec4 color;\n",
"uniform mat4 projection;\n",
"uniform mat4 model;\n",
"uniform mat4 view;\n",
"smooth out vec4 vColor;\n",
"void main(void){\n",
"    gl_Position = projection * view * model  * vec4(position.xyz,  0.6);\n",
"    vColor = color;\n",
"}\n"
};


static const char * geometry_fs_source[] = {
"#version 330 \n",
"smooth in vec4 vColor;\n",
"out vec4 fragColor;\n",
"void main(void) {\n",
"    fragColor = vColor;\n",
"}\n"

};


OGLScene::OGLScene(
                 const cv::Size &winSize):
    bGeometryInitialized(false),
    winSize(winSize),
    geometryIndicesVBO(0),
    geometryVerticesVBO(0),
    geometryVertexArray(0),
    geometryShaderProgramId(-1),
    geometryPositionAttributeLocation(-1),
    geometryColorAttributeLocation(-1),
    geometryModelUniformLocation(-1),
    geometryViewUniformLocation(-1),
    geometryProjectionUniformLocation(-1)
    {}


OGLScene::~OGLScene() {
}

void OGLScene::cleanup() {
     if(bGeometryInitialized) {
        _cleanupGLObjects();
        bGeometryInitialized = false;
     }
}


void OGLScene::_initGeometry(float scale) {
    assert(!bGeometryInitialized);

    const double ac = scale * cos(30*M_PI/180.0);
    const double as = scale * sin(30*M_PI/180.0);
    const double height = scale *sqrt(2.0/3.0);

    geometryVerts[0].location[0] = 0.0f;
    geometryVerts[0].location[1] = scale;
    geometryVerts[0].location[2] = 0.0f;
    geometryVerts[0].location[3] = 1.0f;



    geometryVerts[1].location[0] = -ac;
    geometryVerts[1].location[1] = -as;
    geometryVerts[1].location[2] = 0.0f;
    geometryVerts[1].location[3] = 1.0f;

    geometryVerts[2].location[0] = ac;
    geometryVerts[2].location[1] = -as;
    geometryVerts[2].location[2] = 0.0f;
    geometryVerts[2].location[3] = 1.0f;



    geometryVerts[3].location[0] = 0.0;
    geometryVerts[3].location[1] = 0.0;
    geometryVerts[3].location[2] = -height;
    geometryVerts[3].location[3] = 1.0f;


    geometryVerts[0].color[0] = 1.0f;
    geometryVerts[0].color[1] = 0.0f;
    geometryVerts[0].color[2] = 0.0f;
    geometryVerts[0].color[3] = 1.0f;


    geometryVerts[1].color[0] = 0.0f;
    geometryVerts[1].color[1] = 1.0f;
    geometryVerts[1].color[2] = 0.0f;
    geometryVerts[1].color[3] = 1.0f;


    geometryVerts[2].color[0] = 0.0f;
    geometryVerts[2].color[1] = 0.0f;
    geometryVerts[2].color[2] = 1.0f;
    geometryVerts[2].color[3] = 1.0f;



    geometryVerts[3].color[0] = 1.0f;
    geometryVerts[3].color[1] = 0.0f;
    geometryVerts[3].color[2] = 1.0f;
    geometryVerts[3].color[3] = 1.0f;


    geometryIndices[0] = 0;
    geometryIndices[1] = 1;
    geometryIndices[2] = 2;

    geometryIndices[3] = 0;
    geometryIndices[4] = 3;
    geometryIndices[5] = 1;


    geometryIndices[6] = 0;
    geometryIndices[7] = 2;
    geometryIndices[8] = 3;

    geometryIndices[9] = 1;
    geometryIndices[10] = 3;
    geometryIndices[11] = 2;

}


void OGLScene::_initGLObjects() {
    assert(!bGeometryInitialized);


    glGenVertexArrays(1, &geometryVertexArray);

    glBindVertexArray(geometryVertexArray);

    glGenBuffers(1, &geometryVerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, geometryVerticesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GeometryVertex)*4, (const void *)(&geometryVerts[0].location[0]), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &geometryIndicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometryIndicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*12, (const void *) &geometryIndices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    _setupShaders();
}



void OGLScene::_setupShaders() {
    geometryShaderProgramId = ShaderSupport::makeShaderProgramFromText( geometry_vs_source,  geometry_fs_source );
}

void OGLScene::initIfNeeded() {
    if(!bGeometryInitialized) {
        _initGeometry(0.5f);
        _initGLObjects();
        bGeometryInitialized = true;
    }

}

void OGLScene::_cleanupShaders() {
    ShaderSupport::cleanupShaderProgram( geometryShaderProgramId );
    geometryShaderProgramId = -1;
}


void OGLScene::_getUniformAndAttributeLocations() {
    geometryModelUniformLocation = glGetUniformLocation(geometryShaderProgramId, "model");
    validate(geometryModelUniformLocation >= 0, string("cant find uniform location: ") + string("model"));


    geometryViewUniformLocation = glGetUniformLocation(geometryShaderProgramId, "view");
    validate(geometryViewUniformLocation >= 0, string("cant find uniform location: ") + string("view"));

    geometryProjectionUniformLocation = glGetUniformLocation(geometryShaderProgramId, "projection");
    validate(geometryProjectionUniformLocation >= 0, string("cant find uniform location: ") + string("projection"));



    geometryPositionAttributeLocation = glGetAttribLocation(geometryShaderProgramId, "position");
    validate(geometryPositionAttributeLocation >= 0, string("cant find attribute location: ") + string("position"));


    geometryColorAttributeLocation = glGetAttribLocation(geometryShaderProgramId, "color");
    validate(geometryColorAttributeLocation >= 0, string("cant find attribute location: ") + string("color"));

}

void OGLScene::_cleanupGLObjects() {
    glDeleteBuffers(1, &geometryIndicesVBO);
    glDeleteBuffers(1, &geometryVerticesVBO);
    glDeleteVertexArrays(1, &geometryVertexArray);

    geometryIndicesVBO = 0;
    geometryVerticesVBO = 0;
    geometryVertexArray = 0;
    _cleanupShaders();
}



void OGLScene::_drawCoordAxes(const float axisScale) {
    float lineX[] = {0,0,0,axisScale,0,0};
    float lineY[] = {0,0,0,0,axisScale,0};
    float lineZ[] = {0,0,0,0,0,axisScale};
    
    glLineWidth(2);
    glBegin(GL_LINES);
    
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3fv(lineX);
    glVertex3fv(lineX + 3);
    
    
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3fv(lineY);
    glVertex3fv(lineY + 3);
    
    
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3fv(lineZ);
    glVertex3fv(lineZ + 3);
    
    
    glEnd();
    
}
void OGLScene::drawAugmentedFrame(
        float glModelMatrix[16],
        float glViewMatrix[16],
        float glProjectionMatrix[16]) {
    initIfNeeded();
    
    //use the geometry shader pogram
    glUseProgram(geometryShaderProgramId);
    
    //get uniform and attribute locations
    _getUniformAndAttributeLocations();
    
    //update model, view, projection uniforms
    glUniformMatrix4fv(geometryProjectionUniformLocation, 1, GL_FALSE, &glProjectionMatrix[0]);
    glUniformMatrix4fv(geometryModelUniformLocation, 1, GL_FALSE, &glModelMatrix[0]);
    glUniformMatrix4fv(geometryViewUniformLocation, 1, GL_FALSE, &glViewMatrix[0]);
    
    //vetex buffer object activation
    glBindVertexArray(geometryVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, geometryVerticesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometryIndicesVBO);
    glEnableVertexAttribArray(geometryPositionAttributeLocation);
    glEnableVertexAttribArray(geometryColorAttributeLocation);
    glVertexAttribPointer(
                    geometryPositionAttributeLocation,
                    4, //4 components per location
                    GL_FLOAT, //type
                    GL_FALSE, //normalized
                    sizeof(GeometryVertex),
                    (const void*)(0)
                    );
    
    glVertexAttribPointer(
                      geometryColorAttributeLocation,
                      4, //4 components per color
                      GL_FLOAT, //type
                      GL_TRUE,
                      sizeof(GeometryVertex), //stride
                      (const void *)(16)//offset to color
                      );

    
    //draw elements
    glColor4f(1,1,1,1);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_BYTE, (char*) NULL+0);
    //disable, what was enabled
    glDisableVertexAttribArray(geometryPositionAttributeLocation);
    glDisableVertexAttribArray(geometryColorAttributeLocation);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
