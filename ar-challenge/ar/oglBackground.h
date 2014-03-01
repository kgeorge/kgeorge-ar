
#if !defined(OGL_BACKGROUND_H_)
#define OGL_BACKGROUND_H_
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>


#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>
class OGLDraw;


class OGLBackground {
public:
    
    typedef struct {
        float location[2];
        float textureCoords[2];
    } BackgroundVertex;
    
public:
    OGLBackground();
    ~OGLBackground();
    void        processFrame(cv::Mat &frame);
    void        cleanup();
    void        draw();
    
protected:
    
    void        cleanupGLObjects();
    void        initGLObjects();
    void        initGeometry(int w, int h);
    void        _setupShaders();
    void        _cleanupShaders();
    
    bool        bTextureInitialized;
    unsigned int       backgroundTextureId;
    GLuint          backgroundTextureVerticesVBO;
    GLuint          backgroundTextureIndicesVBO;
    
    BackgroundVertex backgroundTextureVerts[4];
    GLubyte backgroundTextureIndices[6];
    GLuint  backgroundShaderProgramId;
    GLint   backgroundTexUniformLocationInShaderProgram;
    cv::Mat     backgroundImage;
};



#endif //OGL_BACKGROUND_H_
