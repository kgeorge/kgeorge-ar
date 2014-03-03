#include <iostream>

#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

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
    
    glfwSetErrorCallback(error_callback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
     

    window = glfwCreateWindow(800, 600, "check-opengl", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if(glewInit() != GLEW_OK) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(0);
    strGLVersion = glGetString(GL_VERSION);
    cout << "GL_VERSION:" << strGLVersion << endl;
    const GLubyte * strGLShadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    cout << "GL_VERSION:" << strGLShadingLanguageVersion << endl;
    
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
    int win_width = 800;
    int win_height = 600;
    cap >> image;
    //win_width = image.cols;
    //win_height = image.rows;
    const string win_name("kgeorge-ar");
    
    OGLDraw oglDraw = OGLDraw(
                              Size(win_width, win_height),
                              win_name,
                              &perFrameAppData);

    
    
    
    while (!glfwWindowShouldClose(window)) {
        
        cap >> image;
        findPattern(image, perFrameAppData);
        
        Mat temp = image.clone();
        undistort(temp, image, perFrameAppData.intrinsics, perFrameAppData.distortion);
        
        
        
        oglDraw.processFrame( image );
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}
