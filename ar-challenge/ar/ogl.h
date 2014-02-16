
#if !defined(OGL_H_)
#define OGL_H_
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>
#include <Eigen/Dense>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

class OGLDraw;

struct PerFrameAppData {
    bool bValidFrame;
    cv::Mat rvec;
    cv::Mat tvec;
    cv::Mat intrinsics;
    cv::Mat distortion;
    float squareSize;
} ;

class OGLDraw {
    public:
    OGLDraw(const cv::Size &winSize, const std::string &winName, PerFrameAppData *perFrameAppData);
    ~OGLDraw();
    void        cleanup();
    void        updateWindow();
    void        processFrame(cv::Mat & mat);
    void        draw();

    protected:
    void    _drawCameraFrame();
    void    _drawAugmentedFrame();
    void    _drawCoordAxes(const float axisScale);
    void    _buildProjectionMatrix(
        const PerFrameAppData &perFrameAppData,
        double near, double far,
        double left, double right,
        double bottom, double top,
        Eigen::Matrix4d &frustumMatrix);
    void    _buildModelMatrix(const PerFrameAppData &perFrameAppData, cv::Mat &glViewMatrix);
    void    _build_opengl_projection_for_intrinsics(
            Eigen::Matrix4d &frustum,
            int *viewport,
            double alpha,
            double beta,
            double skew,
            double u0,
            double v0,
            int img_width,
            int img_height,
            double near_clip,
            double far_clip );
    
    void _drawTetrahedron(const float axisScale) ;
    
    void _buildViewMatrix(
                                    double ex,
                                    double ey,
                                    double ez,
                                    double lx,
                                    double ly,
                                    double lz,
                                    double ux,
                                    double uy,
                                    double uz,
                                    cv::Mat &glModelMatrix);
    
   
    std::string winName;
    cv::Size    winSize;
    cv::Mat     backgroundImage;
    bool        bTextureInitialized;
    unsigned int       backgroundTextureId;


    //perframe data from app
    public:
    PerFrameAppData *perFrameAppData;

};



#endif
