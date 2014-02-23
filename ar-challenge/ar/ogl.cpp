#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <iostream>
#include <math.h>
#include "ogl.h"

using namespace std;
using namespace cv;


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
        bTextureInitialized(false),
        backgroundTextureId(0),
        perFrameAppData(perFrameAppData){

        namedWindow(winName,  WINDOW_OPENGL);
        resizeWindow(winName, winSize.width, winSize.height);
        setOpenGlContext(winName);

        setOpenGlDrawCallback(winName, OGLDrawCallback, this);
    }


    OGLDraw::~OGLDraw() {
        //cleanup();

    }

    void OGLDraw::draw() {
         glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // Clear entire screen:
         _drawCameraFrame();
         if(perFrameAppData->bValidFrame) {
            _drawAugmentedFrame();
         }
         glFlush();
    }

    void OGLDraw::cleanup() {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &backgroundTextureId);
        bTextureInitialized = false;
        backgroundTextureId = 0;
        setOpenGlDrawCallback(winName, 0, 0);
        destroyAllWindows();
    }

    void OGLDraw::processFrame( Mat &frame) {
        frame.copyTo(backgroundImage);

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

    void OGLDraw::_drawCameraFrame() {
        if (!bTextureInitialized){
              glGenTextures(1, &backgroundTextureId);
              glBindTexture(GL_TEXTURE_2D, backgroundTextureId);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
              bTextureInitialized = true;
        }

        const float fw = backgroundImage.cols;
        const float fh = backgroundImage.rows;
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, backgroundTextureId);
        switch(backgroundImage.channels()) {
            case 3:
               glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fw, fh, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, backgroundImage.data);
               break;
            case 4:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fw, fh, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, backgroundImage.data);
                break;

            default:
                throw runtime_error("unsupported image format");

        }

        const GLfloat bgTextureVertices[] = { 0, 0, fw, 0, 0, fh, fw, fh };
        const GLfloat bgTextureCoords[] = { 0, 1, 1, 1, 0, 0, 1, 0 };
        const GLfloat proj[]              = { 2.0f/fw, 0, 0, 0, 0, 2.0f/fh, 0, 0, 0, 0, 1, 0, -1, -1, 0, 1 };
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(proj);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, backgroundTextureId);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);


        glVertexPointer(2, GL_FLOAT, 0, bgTextureVertices);
        glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);
        glColor4f(1,1,1,1);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);

    }

    void OGLDraw::_drawAugmentedFrame() {
         Eigen::Matrix4d frustumMatrix;
         int vp[4];
        OGLDraw *pogl = this;
        //double near =0.1, far =100.0, left =0, right =winSize.width, top=winSize.height, bottom=0;
        double near =0.1, far =100.0, left =-winSize.width/2.0, right =winSize.width/2.0, top=winSize.height/2.0, bottom=-winSize.height/2.0;
        //double near =0.1, far =100.0, left =0, right =16, top=12.8, bottom=0;
        double alpha = perFrameAppData->intrinsics.at<double>(0,0);
        double beta = perFrameAppData->intrinsics.at<double>(1,1);
        double x0 = perFrameAppData->intrinsics.at<double>(0,2);
        double y0 = perFrameAppData->intrinsics.at<double>(1,2);
        
        _buildProjectionMatrix(
            *perFrameAppData,
            near, far,
            left, right,
            bottom, top,
            frustumMatrix);
        
        
        /*
        _build_opengl_projection_for_intrinsics(
            frustumMatrix,
            vp,
            alpha,
            beta,
            0.0,
            x0,
            y0,
            right,
            top,
            near,
            far );
        */
         cv::Mat glModelMatrix(4,4, CV_64F, 0);
         cv::Mat glViewMatrix(4,4, CV_64F, 0);
        _buildModelMatrix(*perFrameAppData, glModelMatrix);
        _buildViewMatrix(0,0,3,0,0,0,0,1,0, glViewMatrix);
        Mat rotMat(3,3, CV_32FC1);
        cv::Rodrigues( perFrameAppData->rvec, rotMat );
        
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(&frustumMatrix(0,0));
        //gluPerspective(45.0, (double)winSize.width / winSize.height, 0.1, 100.0);
        glMatrixMode(GL_MODELVIEW);
        
        glMultMatrixd(&glViewMatrix.at<double>(0,0));

        glLoadMatrixd(&glModelMatrix.at<double>(0, 0));        //gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);
        //_drawTetrahedron(0.5f);
        _drawCoordAxes(0.5f);
    }
    /**
 @brief basic function to produce an OpenGL projection matrix and associated viewport parameters
 which match a given set of camera intrinsics. This is currently written for the Eigen linear
 algebra library, however it should be straightforward to port to any 4x4 matrix class.
 @param[out] frustum Eigen::Matrix4d projection matrix.  Eigen stores these matrices in column-major (i.e. OpenGL) order.
 @param[out] viewport 4-component OpenGL viewport values, as might be retrieved by glGetIntegerv( GL_VIEWPORT, &viewport[0] )
 @param[in]  alpha x-axis focal length, from camera intrinsic matrix
 @param[in]  alpha y-axis focal length, from camera intrinsic matrix
 @param[in]  skew  x and y axis skew, from camera intrinsic matrix
 @param[in]  u0 image origin x-coordinate, from camera intrinsic matrix
 @param[in]  v0 image origin y-coordinate, from camera intrinsic matrix
 @param[in]  img_width image width, in pixels
 @param[in]  img_height image height, in pixels
 @param[in]  near_clip near clipping plane z-location, can be set arbitrarily > 0, controls the mapping of z-coordinates for OpenGL
 @param[in]  far_clip  far clipping plane z-location, can be set arbitrarily > near_clip, controls the mapping of z-coordinate for OpenGL
*/
void OGLDraw::_build_opengl_projection_for_intrinsics(
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
    double far_clip ){

    // These parameters define the final viewport that is rendered into by
    // the camera.
/*
    double L = 0.0;
    double R = img_width;
    double B = 0.0;
    double T = img_height/2.0;
*/

    double L = -img_width/2.0;
    double R = img_width/2.0;
    double B = -img_height/2.0;
    double T = img_height/2.0;

    // near and far clipping planes, these only matter for the mapping from
    // world-space z-coordinate into the depth coordinate for OpenGL
    double N = near_clip;
    double F = far_clip;

    // set the viewport parameters
    viewport[0] = L;
    viewport[1] = B;
    viewport[2] = R-L;
    viewport[3] = T-B;

    // construct an orthographic matrix which maps from projected
    // coordinates to normalized device coordinates in the range
    // [-1, 1].  OpenGL then maps coordinates in NDC to the current
    // viewport
    Eigen::Matrix4d ortho = Eigen::Matrix4d::Zero();
    ortho(0,0) =  2.0/(R-L); ortho(0,3) = -(R+L)/(R-L);
    ortho(1,1) =  2.0/(T-B); ortho(1,3) = -(T+B)/(T-B);
    ortho(2,2) = -2.0/(F-N); ortho(2,3) = -(F+N)/(F-N);
    ortho(3,3) =  1.0;

    // construct a projection matrix, this is identical to the
    // projection matrix computed for the intrinsicx, except an
    // additional row is inserted to map the z-coordinate to
    // OpenGL.
    Eigen::Matrix4d tproj = Eigen::Matrix4d::Zero();
    tproj(0,0) = alpha; tproj(0,1) = skew; tproj(0,2) = -u0;
                        tproj(1,1) = beta; tproj(1,2) = -v0;
                                           tproj(2,2) = (N+F); tproj(2,3) = N*F;
                                           tproj(3,2) = -1.0;

    // resulting OpenGL frustum is the product of the orthographic
    // mapping to normalized device coordinates and the augmented
    // camera intrinsic matrix
    frustum = ortho*tproj;
}
    //http://ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/
    void OGLDraw::_buildProjectionMatrix(
        const  PerFrameAppData &perFrameAppData,
        double near, double far,
        double left, double right,
        double bottom, double top,
        Eigen::Matrix4d &frustumMatrix) {
        double alpha = perFrameAppData.intrinsics.at<double>(0,0);
        double beta = perFrameAppData.intrinsics.at<double>(1,1);
        double x0 = perFrameAppData.intrinsics.at<double>(0,2);
        double y0 = perFrameAppData.intrinsics.at<double>(1,2);
        x0=0.0;
        y0=0.0;
        CV_Assert( alpha > 0 );
        CV_Assert( beta >  0 );
        CV_Assert( right > left  );
        CV_Assert( top > bottom  );
        CV_Assert( far > near  );

        double left_modified = (near/alpha) * left -  x0;
        double right_modified = (near/alpha) * right - x0;
        double bottom_modified = (near/beta) * bottom - y0;
        double top_modified = (near/beta) * top  - y0;

        frustumMatrix.setZero();
        frustumMatrix(0,0) = 2.0 * near /( right_modified - left_modified );
        frustumMatrix(0,2) = (right_modified + left_modified ) / (right_modified - left_modified);
        frustumMatrix(1,1) = 2.0 * near / (top_modified - bottom_modified);
        frustumMatrix(1,2) = (top_modified + bottom_modified) / (top_modified - bottom_modified);
        frustumMatrix(2,2) = -(far + near)/ (far - near);
        frustumMatrix(2,3) = -2.0 * far * near / (far - near);
        frustumMatrix(3,2) = -1.0;
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

    void OGLDraw::_buildViewMatrix(
                                     double ex,
                                     double ey,
                                     double ez,
                                     double lx,
                                     double ly,
                                     double lz,
                                     double ux,
                                     double uy,
                                     double uz,
                                     cv::Mat &glViewMatrix) {
    
        cv::Mat tempViewMatrix(4,4,CV_64F, Scalar(0));
        cv::Vec<double, 3> eye;
        eye[0] = ex; eye[1] = ey; eye[2] = ez;
        cv::Vec<double, 3> look;
        look[0] = lx; look[1] = ly; look[2] = lz;
        cv::Vec<double, 3> up(3, 1, CV_64F);
        up[0] = ux; up[1] = uy; up[2] = uz;
        cv::Vec<double, 3> backward = eye - look;
        backward *= 1.0/(norm(backward) + 0.000001);
        cv::Vec<double, 3> right;
        cross(up, backward, right);
        right *= 1.0/(norm(right) + 0.000001);
        cv::Vec<double, 3> yup;
        cross(backward, right, yup);
        yup *= 1.0/(norm(yup) + 0.000001);
        
        tempViewMatrix.at<double>(0,0) = right[0];
        tempViewMatrix.at<double>(0,1) = yup[0];
        tempViewMatrix.at<double>(0,2) = backward[0];
        tempViewMatrix.at<double>(0,3) = ex;
        tempViewMatrix.at<double>(1,0) = right[1];
        tempViewMatrix.at<double>(1,1) = yup[1];
        tempViewMatrix.at<double>(1,2) = backward[1];
        tempViewMatrix.at<double>(1,3) = ey;
        tempViewMatrix.at<double>(2,0) = right[2];
        tempViewMatrix.at<double>(2,1) = yup[2];
        tempViewMatrix.at<double>(2,2) = backward[2];
        tempViewMatrix.at<double>(2,3) = ez;
        
        tempViewMatrix.at<double>(3,0) = 0;
        tempViewMatrix.at<double>(3,1) = 0;
        tempViewMatrix.at<double>(3,2) = 0;
        tempViewMatrix.at<double>(3,3) = 1.0;
        
        cv::Mat tempViewMatrixInv = tempViewMatrix.inv();
        
        cv::transpose(tempViewMatrixInv, glViewMatrix);
    }

    void OGLDraw::updateWindow() {
        cv::updateWindow(winName);
    }
