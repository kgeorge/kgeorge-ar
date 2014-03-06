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
#include "ogl.h"
#include "kgUtil.h"


using namespace std;
using namespace cv;

void convert_from_opencv_4x464F_to_opengl_4x4f(cv::Mat &mat, float openglMat[16]) {
    auto computeIndex = [](int i, int j) { return (i*4 + j); };
    assert(mat.type() == CV_64F && mat.size() == Size(4,4));
    openglMat[computeIndex(0,0)] = mat.at<double>(0,0);
    openglMat[computeIndex(0,1)] = mat.at<double>(0,1);
    openglMat[computeIndex(0,2)] = mat.at<double>(0,2);
    openglMat[computeIndex(0,3)] = mat.at<double>(0,3);
    
    
    
    openglMat[computeIndex(1,0)] = mat.at<double>(1,0);
    openglMat[computeIndex(1,1)] = mat.at<double>(1,1);
    openglMat[computeIndex(1,2)] = mat.at<double>(1,2);
    openglMat[computeIndex(1,3)] = mat.at<double>(1,3);
    
    
    openglMat[computeIndex(2,0)] = mat.at<double>(2,0);
    openglMat[computeIndex(2,1)] = mat.at<double>(2,1);
    openglMat[computeIndex(2,2)] = mat.at<double>(2,2);
    openglMat[computeIndex(2,3)] = mat.at<double>(2,3);
    
    
    
    openglMat[computeIndex(3,0)] = mat.at<double>(3,0);
    openglMat[computeIndex(3,1)] = mat.at<double>(3,1);
    openglMat[computeIndex(3,2)] = mat.at<double>(3,2);
    openglMat[computeIndex(3,3)] = mat.at<double>(3,3);
}

void OGLDrawCallback(void* userdata) {
    OGLDraw *draw = reinterpret_cast<OGLDraw *> (userdata);
    draw->draw();
}

OGLDraw::OGLDraw(
                 const cv::Size &winSize,
                 const std::string &winName,
                 PerFrameAppData *perFrameAppData):
winName(winName),
winSize(winSize),
augmentedScene(winSize),
perFrameAppData(perFrameAppData){}


OGLDraw::~OGLDraw() {
}

void OGLDraw::draw() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // Clear entire screen:


    background.draw();
    if(perFrameAppData->bValidFrame) {
       _drawAugmentedFrame();
    }
}

void OGLDraw::cleanup() {
    augmentedScene.cleanup();
    background.cleanup();
    setOpenGlDrawCallback(winName, 0, 0);
    destroyAllWindows();
}

void OGLDraw::processFrame( Mat &frame) {
    background.processFrame(frame);
    draw();
}



void OGLDraw::_drawTetrahedron(const float axisScale) {
    const double ac = axisScale * cos(30*M_PI/180.0);
    const double as = axisScale * sin(30*M_PI/180.0);
    const double height = axisScale *sqrt(2.0/3.0);
    
    double A[] = {0,axisScale,0};
    double B[] = {ac, as, 0};
    double C[] = {-ac, -as, 0};
    double  D[] = {0,0, -height};
    
    glLineWidth(4);
    glBegin(GL_LINES);
    
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3dv(A);
    glVertex3dv(B);
    
    
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3dv(B);
    glVertex3dv(C);
    
    
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3dv(C);
    glVertex3dv(A);
    
    
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3dv(A);
    glVertex3dv(D);
    
    
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3dv(B);
    glVertex3dv(D);
    
    
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3dv(C);
    glVertex3dv(D);
    
    
    
    
    glEnd();
    
}



void OGLDraw::_drawCoordAxes(const float axisScale) {
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
void OGLDraw::_drawAugmentedFrame() {
    cv::Mat cvFrustumMatrix(4, 4, CV_64F, Scalar(0));
    double near =0.1, far =100.0;
    _buildProjectionMatrix(
                           *perFrameAppData,
                           near, far,
                           cvFrustumMatrix);
    cv::Mat cvModelMatrix(4,4, CV_64F, 0);
    cv::Mat cvViewMatrix(4,4, CV_64F, 0);
    _buildModelMatrix(*perFrameAppData, cvModelMatrix);
    _buildViewMatrix(cvViewMatrix);
    
    
    assert(!glewGetExtension("GL_ARB_gpu_shader_fp64"));
    float glFrustumMatrix[16];
    float glModelMatrix[16];
    float glViewMatrix[16];
    convert_from_opencv_4x464F_to_opengl_4x4f(cvFrustumMatrix, glFrustumMatrix );
    convert_from_opencv_4x464F_to_opengl_4x4f(cvModelMatrix, glModelMatrix );
    convert_from_opencv_4x464F_to_opengl_4x4f(cvViewMatrix, glViewMatrix );
    
    augmentedScene.drawAugmentedFrame( glModelMatrix, glViewMatrix, glFrustumMatrix );
}

//http://ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/
void OGLDraw::_buildProjectionMatrix(
                                     const  PerFrameAppData &perFrameAppData,
                                     double near, double far,
                                     cv::Mat &frustumMatrix) {
    Mat tempFrustumMatrix(4, 4, CV_64F, Scalar(0));
    double alpha = perFrameAppData.intrinsics.at<double>(0,0);
    double beta = perFrameAppData.intrinsics.at<double>(1,1);
    double x0 = perFrameAppData.intrinsics.at<double>(0,2);
    double y0 = perFrameAppData.intrinsics.at<double>(1,2);
    CV_Assert( alpha > 0 );
    CV_Assert( beta >  0 );
    CV_Assert( far > near  );
    
    double left_modified = -(near/alpha) * x0;
    double right_modified =  (near/alpha) * x0;
    double bottom_modified = -(near/beta) * y0;
    double top_modified = (near/beta) * y0;
    //cout << endl << "left modified: " << left_modified << "  bottom modified " << bottom_modified << endl;
    
    tempFrustumMatrix.at<double>(0,0) = 2.0  * near /( right_modified - left_modified );
    tempFrustumMatrix.at<double>(0,2) = (right_modified + left_modified) /( right_modified - left_modified );
    tempFrustumMatrix.at<double>(1,1) = 2.0 * near  / (top_modified - bottom_modified);
    tempFrustumMatrix.at<double>(1,2) = (top_modified + bottom_modified)/  (top_modified - bottom_modified);
    tempFrustumMatrix.at<double>(2,2) = -(far + near) / (far - near);
    tempFrustumMatrix.at<double>(2,3) = -2 * far * near / (far - near);
    tempFrustumMatrix.at<double>(3,2) = -1.0;
    //cout << "frsutum mtrix: " << tempFrustumMatrix << endl;
    cv::transpose(tempFrustumMatrix, frustumMatrix);
}

void OGLDraw::_buildModelMatrix(const PerFrameAppData &perFrameAppData, cv::Mat &glModelMatrix)
{
    cv::Mat rvec(perFrameAppData.rvec);
    cv::Mat tvec(perFrameAppData.tvec);
    cv::Mat rotation(3, 3, CV_64F, Scalar(0)), tempModelMatrix(4, 4, CV_64F, Scalar(0)), cvToOgl(4, 4, CV_64F, Scalar(0));
    glModelMatrix = cv::Mat(4,4, CV_64F, Scalar(0));
    cv::Rodrigues(rvec, rotation);
    tempModelMatrix(cv::Range(0, 3), cv::Range(0, 3)) = rotation * 1;
    tempModelMatrix(cv::Range(0, 3), cv::Range(3, 4)) = tvec * 1;
    *(tempModelMatrix.ptr<double>(3,3)) = 1.0;
    cvToOgl.at<double>(0,0) =   1.0;
    cvToOgl.at<double>(1,1) =   -1.0;
    cvToOgl.at<double>(2,2) =   -1.0;
    cvToOgl.at<double>(3,3) =   1.0;
    cout << "rvec: " << rotation << endl;
    cout << "tvec: " << tvec << endl;
    tempModelMatrix = cvToOgl * tempModelMatrix;
    cv::transpose(tempModelMatrix, glModelMatrix);
}

void cross (const cv::Vec<double,3> &a, const cv::Vec<double, 3> &b, cv::Vec<double, 3> &out) {
    double ax = a[0];
    double ay = a[1];
    double az = a[2];
    double bx = b[0];
    double by = b[1];
    double bz = b[2];
    out[0] = ay*bz - az*by;
    out[1] = az*bx - ax*bz;
    out[2] = ax*by - ay*bz;
}

void OGLDraw::_buildViewMatrix( cv::Mat &glViewMatrix) {
    Mat tempViewMatrix(4, 4, CV_64F, Scalar(0));
    tempViewMatrix.at<double>(0,0) = 1.0;
    tempViewMatrix.at<double>(1,1) = 1.0;
    tempViewMatrix.at<double>(2,2) = 1.0;
    tempViewMatrix.at<double>(3,3) = 1.0;
    
    cv::Mat tempViewMatrixInv = tempViewMatrix.inv();
    cv::transpose(tempViewMatrixInv, glViewMatrix);
}

void OGLDraw::updateWindow() {
    cv::updateWindow(winName);
}
