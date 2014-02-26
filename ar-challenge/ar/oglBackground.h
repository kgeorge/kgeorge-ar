
#if !defined(OGL_BACKGROUND_H_)
#define OGL_BACKGROUND_H_
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

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

    bool        bTextureInitialized;
    unsigned int       backgroundTextureId;
    GLuint          backgroundTextureVerticesVBO;
    GLuint          backgroundTextureIndicesVBO;

    BackgroundVertex backgroundTextureVerts[4];
    GLubyte backgroundTextureIndices[6];
    
    cv::Mat     backgroundImage;
};



#endif //OGL_BACKGROUND_H_
