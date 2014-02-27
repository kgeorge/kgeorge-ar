#include <iostream>
#include <GLFW/glfw3.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "ogl.h"
#include "findPattern.h"


using namespace std;
using namespace cv;

void error_callback(int errnum, const char *desc) {
    cout << "Error: " << desc << endl;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;
    
    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
    }
}

int main(int argc, char **argv) {
    GLFWwindow* window;
    const GLubyte * strGLVersion;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    //
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    window = glfwCreateWindow(400, 300, "check-opengl", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(0);
    strGLVersion = glGetString(GL_VERSION);
    cout << "GL_VERSION:" << strGLVersion << endl;
    
    
    
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
    //win_width = image.cols;
    //win_height = image.rows;
    const string win_name("kgeorge-ar");
    
    
    
    
    while (!glfwWindowShouldClose(window)) {
        
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

        
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}
