
#if !defined(OGL_SCENE_H_)
#define OGL_SCENE_H_
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>

#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>



class OGLScene {
public:

    typedef struct {
        float location[4];
        float color[4];
    } GeometryVertex;

    OGLScene(const cv::Size &winSize);
    ~OGLScene();
    void        cleanup();
    void    drawAugmentedFrame(
        float glModelMatrix[16],
        float glViewMatrix[16],
        float glProjectionMatrix[16]
    );
    void initIfNeeded();

protected:
    void    _drawCoordAxes(const float axisScale);

    void _initGeometry(float scale);
    void _initGLObjects();
    void _cleanupGLObjects();
    void _setupShaders();
    void _cleanupShaders();
    void _getUniformAndAttributeLocations();
    cv::Size    winSize;

    bool            bGeometryInitialized;
    GeometryVertex  geometryVerts[4];
    GLubyte         geometryIndices[12];


    GLuint          geometryVertexArray;
    GLuint          geometryVerticesVBO;
    GLuint          geometryIndicesVBO;


    GLuint  geometryShaderProgramId;

    GLint geometryPositionAttributeLocation;
    GLint geometryColorAttributeLocation;
    GLint geometryModelUniformLocation;
    GLint geometryViewUniformLocation;
    GLint geometryProjectionUniformLocation;



};



#endif //OGL_SCENE_H_
