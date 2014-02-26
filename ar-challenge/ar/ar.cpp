#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <iostream>
#if defined(__APPLE__)
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else 

    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif
#include "ogl.h"
#include "findPattern.h"


const char ESC_KEY = 27;
using namespace std;
using namespace cv;


int main(int argc, char** argv) {
    double delay = 30.0;
    VideoCapture cap(0);
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    PerFrameAppData perFrameAppData;
    FileStorage fs("out_camera_data.xml", FileStorage::READ );
    
    fs["Camera_Matrix"] >> perFrameAppData.intrinsics;
    fs["Distortion_Coefficients"] >> perFrameAppData.distortion;
    fs["square_Size"] >> perFrameAppData.squareSize;
    
    cout << "intrinsics" << perFrameAppData.intrinsics << endl;
    cout << "distortion coefficients" << perFrameAppData.distortion << endl;
    
    Mat image;
    RNG rng(12345);
    int win_width = 400;
    int win_height = 300;
    cap >> image;
    win_width = image.cols;
    win_height = image.rows;

    const string win_name("kgeorge-ar");
    OGLDraw oglDraw = OGLDraw(
        Size(win_width, win_height),
        win_name,
        &perFrameAppData);

    Scalar color_mark = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
    for (;;) {
        cap >> image;
        findPattern(image, perFrameAppData);
        
        Mat temp = image.clone();
        undistort(temp, image, perFrameAppData.intrinsics, perFrameAppData.distortion);
        
        oglDraw.processFrame(  image);
        oglDraw.updateWindow();
        
        //key behavior
        char key = (char)waitKey(delay);
        if (key == ESC_KEY ) {
            break;
        }
    }
    oglDraw.cleanup();

    return 0;
}