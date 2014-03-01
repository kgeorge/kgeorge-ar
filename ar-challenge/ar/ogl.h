
#if !defined(OGL_H_)
#define OGL_H_
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>

#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>

#include "oglBackground.h"


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
                                   cv::Mat &frustumMatrix);
    void    _buildModelMatrix(const PerFrameAppData &perFrameAppData, cv::Mat &glViewMatrix);
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
    OGLBackground   background;
    //perframe data from app
public:
    PerFrameAppData *perFrameAppData;
    
};



#endif
