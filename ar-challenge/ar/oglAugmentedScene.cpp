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
#include "oglAugmentedScene.h"
#include "kgShaderSupport.h"

using namespace std;
using namespace cv;


static const char * augmentedSceneSource_vs[] = {
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


static const char * augmentedSceneSource_fs[] = {
"#version 330 \n",
"smooth in vec4 vColor;\n",
"out vec4 fragColor;\n",
"void main(void) {\n",
"    fragColor = vColor;\n",
"}\n"

};


OGLAugmentedScene::OGLAugmentedScene(float squareSize):
    squareSize(squareSize),
    bAugmentedSceneIntialized(false),
    augmentedSceneIndicesVBO(0),
    augmentedSceneVerticesVBO(0),
    augmentedSceneVertexArray(0),
    augmentedSceneProgramId(-1),
    augmentedScenePositionAttributeLocation(-1),
    augmentedSceneColorAttributeLocation(-1),
    augmentedSceneModelUniformLocation(-1),
    augmentedSceneViewUniformLocation(-1),
    augmentedSceneProjectionUniformLocation(-1)
    {}


OGLAugmentedScene::~OGLAugmentedScene() {
}

void OGLAugmentedScene::cleanup() {
     if(bAugmentedSceneIntialized) {
        _cleanupGLObjects();
        bAugmentedSceneIntialized = false;
     }
}


void OGLAugmentedScene::_initGeometry(float scale) {
    assert(!bAugmentedSceneIntialized);

    const double ac = scale * cos(30*M_PI/180.0);
    const double as = scale * sin(30*M_PI/180.0);
    const double height = scale *sqrt(2.0/3.0);

    augmentedSceneVertices[0].location[0] = 0.0f;
    augmentedSceneVertices[0].location[1] = scale;
    augmentedSceneVertices[0].location[2] = 0.0f;
    augmentedSceneVertices[0].location[3] = 1.0f;



    augmentedSceneVertices[1].location[0] = -ac;
    augmentedSceneVertices[1].location[1] = -as;
    augmentedSceneVertices[1].location[2] = 0.0f;
    augmentedSceneVertices[1].location[3] = 1.0f;

    augmentedSceneVertices[2].location[0] = ac;
    augmentedSceneVertices[2].location[1] = -as;
    augmentedSceneVertices[2].location[2] = 0.0f;
    augmentedSceneVertices[2].location[3] = 1.0f;



    augmentedSceneVertices[3].location[0] = 0.0;
    augmentedSceneVertices[3].location[1] = 0.0;
    augmentedSceneVertices[3].location[2] = -height;
    augmentedSceneVertices[3].location[3] = 1.0f;


    augmentedSceneVertices[0].color[0] = 1.0f;
    augmentedSceneVertices[0].color[1] = 0.0f;
    augmentedSceneVertices[0].color[2] = 0.0f;
    augmentedSceneVertices[0].color[3] = 1.0f;


    augmentedSceneVertices[1].color[0] = 0.0f;
    augmentedSceneVertices[1].color[1] = 1.0f;
    augmentedSceneVertices[1].color[2] = 0.0f;
    augmentedSceneVertices[1].color[3] = 1.0f;


    augmentedSceneVertices[2].color[0] = 0.0f;
    augmentedSceneVertices[2].color[1] = 0.0f;
    augmentedSceneVertices[2].color[2] = 1.0f;
    augmentedSceneVertices[2].color[3] = 1.0f;



    augmentedSceneVertices[3].color[0] = 1.0f;
    augmentedSceneVertices[3].color[1] = 0.0f;
    augmentedSceneVertices[3].color[2] = 1.0f;
    augmentedSceneVertices[3].color[3] = 1.0f;


    augmentedSceneIndices[0] = 0;
    augmentedSceneIndices[1] = 1;
    augmentedSceneIndices[2] = 2;

    augmentedSceneIndices[3] = 0;
    augmentedSceneIndices[4] = 3;
    augmentedSceneIndices[5] = 1;


    augmentedSceneIndices[6] = 0;
    augmentedSceneIndices[7] = 2;
    augmentedSceneIndices[8] = 3;

    augmentedSceneIndices[9] = 1;
    augmentedSceneIndices[10] = 3;
    augmentedSceneIndices[11] = 2;

}


void OGLAugmentedScene::_initGLObjects() {
    assert(!bAugmentedSceneIntialized);


    glGenVertexArrays(1, &augmentedSceneVertexArray);

    glBindVertexArray(augmentedSceneVertexArray);

    glGenBuffers(1, &augmentedSceneVerticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, augmentedSceneVerticesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(AugmentedSceneVertex)*4, (const void *)(&augmentedSceneVertices[0].location[0]), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &augmentedSceneIndicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, augmentedSceneIndicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)*12, (const void *) &augmentedSceneIndices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    _setupShaders();
}



void OGLAugmentedScene::_setupShaders() {
    augmentedSceneProgramId = ShaderSupport::makeShaderProgramFromText( augmentedSceneSource_vs,  augmentedSceneSource_fs );
}

void OGLAugmentedScene::initIfNeeded() {
    if(!bAugmentedSceneIntialized) {
        _initGeometry(squareSize/2.0f);
        _initGLObjects();
        bAugmentedSceneIntialized = true;
    }

}

void OGLAugmentedScene::_cleanupShaders() {
    ShaderSupport::cleanupShaderProgram( augmentedSceneProgramId );
    augmentedSceneProgramId = -1;
}


void OGLAugmentedScene::_getUniformAndAttributeLocations() {
    augmentedSceneModelUniformLocation = glGetUniformLocation(augmentedSceneProgramId, "model");
    validate(augmentedSceneModelUniformLocation >= 0, string("cant find uniform location: ") + string("model"));


    augmentedSceneViewUniformLocation = glGetUniformLocation(augmentedSceneProgramId, "view");
    validate(augmentedSceneViewUniformLocation >= 0, string("cant find uniform location: ") + string("view"));

    augmentedSceneProjectionUniformLocation = glGetUniformLocation(augmentedSceneProgramId, "projection");
    validate(augmentedSceneProjectionUniformLocation >= 0, string("cant find uniform location: ") + string("projection"));



    augmentedScenePositionAttributeLocation = glGetAttribLocation(augmentedSceneProgramId, "position");
    validate(augmentedScenePositionAttributeLocation >= 0, string("cant find attribute location: ") + string("position"));


    augmentedSceneColorAttributeLocation = glGetAttribLocation(augmentedSceneProgramId, "color");
    validate(augmentedSceneColorAttributeLocation >= 0, string("cant find attribute location: ") + string("color"));

}

void OGLAugmentedScene::_cleanupGLObjects() {
    glDeleteBuffers(1, &augmentedSceneIndicesVBO);
    glDeleteBuffers(1, &augmentedSceneVerticesVBO);
    glDeleteVertexArrays(1, &augmentedSceneVertexArray);

    augmentedSceneIndicesVBO = 0;
    augmentedSceneVerticesVBO = 0;
    augmentedSceneVertexArray = 0;
    _cleanupShaders();
}



void OGLAugmentedScene::_drawCoordAxes(const float axisScale) {
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
void OGLAugmentedScene::drawAugmentedFrame(
        float glModelMatrix[16],
        float glViewMatrix[16],
        float glProjectionMatrix[16]) {
    initIfNeeded();
    
    //use the geometry shader pogram
    glUseProgram(augmentedSceneProgramId);
    
    //get uniform and attribute locations
    _getUniformAndAttributeLocations();
    
    //update model, view, projection uniforms
    glUniformMatrix4fv(augmentedSceneProjectionUniformLocation, 1, GL_FALSE, &glProjectionMatrix[0]);
    glUniformMatrix4fv(augmentedSceneModelUniformLocation, 1, GL_FALSE, &glModelMatrix[0]);
    glUniformMatrix4fv(augmentedSceneViewUniformLocation, 1, GL_FALSE, &glViewMatrix[0]);
    
    //vetex buffer object activation
    glBindVertexArray(augmentedSceneVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, augmentedSceneVerticesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, augmentedSceneIndicesVBO);
    glEnableVertexAttribArray(augmentedScenePositionAttributeLocation);
    glEnableVertexAttribArray(augmentedSceneColorAttributeLocation);
    glVertexAttribPointer(
                    augmentedScenePositionAttributeLocation,
                    4, //4 components per location
                    GL_FLOAT, //type
                    GL_FALSE, //normalized
                    sizeof(AugmentedSceneVertex),
                    (const void*)(0)
                    );
    
    glVertexAttribPointer(
                      augmentedSceneColorAttributeLocation,
                      4, //4 components per color
                      GL_FLOAT, //type
                      GL_TRUE,
                      sizeof(AugmentedSceneVertex), //stride
                      (const void *)(16)//offset to color
                      );

    
    //draw elements
    glColor4f(1,1,1,1);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_BYTE, (char*) NULL+0);
    //disable, what was enabled
    glDisableVertexAttribArray(augmentedScenePositionAttributeLocation);
    glDisableVertexAttribArray(augmentedSceneColorAttributeLocation);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
