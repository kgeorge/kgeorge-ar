
#if !defined(OGL_H_)
#define OGL_H_
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>

#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>

#include "kgBackground.h"
#include "oglAugmentedScene.h"


class OGLMain;

struct PerFrameAppData {
    bool bValidFrame;
    cv::Mat rvec;
    cv::Mat tvec;
    cv::Mat intrinsics;
    cv::Mat distortion;
    float squareSize;
} ;

class OGLMain {
public:
    OGLMain(const cv::Size &winSize, const std::string &winName, PerFrameAppData *perFrameAppData);
    ~OGLMain();
    void        cleanup();
    void        processFrame(cv::Mat & mat);
    void        draw();
    
protected:
    void    _drawCameraFrame();
    void    _drawAugmentedFrame();
    void    _buildProjectionMatrix(
                                   const PerFrameAppData &perFrameAppData,
                                   double near, double far,
                                   cv::Mat &frustumMatrix);
    void    _buildModelMatrix(const PerFrameAppData &perFrameAppData, cv::Mat &glViewMatrix);
    void    _buildViewMatrix(cv::Mat &glModelMatrix);
    
    
    std::string winName;
    cv::Size    winSize;
    OGLBackground   background;
    OGLAugmentedScene     augmentedScene;
    //perframe data from app
public:
    PerFrameAppData *perFrameAppData;
    
};



#endif
