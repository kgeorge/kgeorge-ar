
#if !defined(OGL_SCENE_H_)
#define OGL_SCENE_H_
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>

#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>



class OGLAugmentedScene {
public:

    typedef struct {
        float location[4];
        float color[4];
    } AugmentedSceneVertex;

    OGLAugmentedScene(float squareSize);
    ~OGLAugmentedScene();
    void        cleanup();
    void    drawAugmentedFrame(
        float glModelMatrix[16],
        float glViewMatrix[16],
        float glProjectionMatrix[16]
    );
    void initIfNeeded();

protected:
    //private member funs
    void    _drawCoordAxes(const float axisScale);

    void _initGeometry(float scale);
    void _initGLObjects();
    void _cleanupGLObjects();
    void _setupShaders();
    void _cleanupShaders();
    void _getUniformAndAttributeLocations();


protected:
    //member vars
    float squareSize;
    bool            bAugmentedSceneIntialized;
    AugmentedSceneVertex  augmentedSceneVertices[4];
    GLubyte         augmentedSceneIndices[12];


    GLuint          augmentedSceneVertexArray;
    GLuint          augmentedSceneVerticesVBO;
    GLuint          augmentedSceneIndicesVBO;


    GLuint  augmentedSceneProgramId;
    GLint augmentedScenePositionAttributeLocation;
    GLint augmentedSceneColorAttributeLocation;
    GLint augmentedSceneModelUniformLocation;
    GLint augmentedSceneViewUniformLocation;
    GLint augmentedSceneProjectionUniformLocation;



};



#endif //OGL_SCENE_H_
