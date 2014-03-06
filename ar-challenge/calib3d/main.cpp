
#include <iostream>
#include <vector>
#include <functional>
#include <glew.h>
#define GLFW_INCLUDE_GL3  /* don't drag in legacy GL headers. */
#define GLFW_NO_GLU       /* don't drag in the old GLU lib - unless you must. */

#include <GLFW/glfw3.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "settings.h"
#include "kgClient.h"
#include "calib3d.h"


using namespace cv;
using namespace std;


void error_callback(int errnum, const char *desc) {
    cout << "Error: " << desc << endl;
}


vector<std::function<void(char)>> handleToKeyboardFun;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    if(handleToKeyboardFun.size() > 0) {
        assert(handleToKeyboardFun.size() ==1);
        handleToKeyboardFun[0](key);
    }
    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
    }
}

int main( int argc, char *argv[] ) {
    
    Settings *ps = new Settings();
    const string inputSettingsFile = argc > 1 ? argv[1] : "default.xml";
    FileStorage fs(inputSettingsFile, FileStorage::READ);
    if( !fs.isOpened() ) {
        cout << "Could not open the config file: \"" << inputSettingsFile << "\"" << endl;
        return -1;
        
    }
    fs["Settings"] >> *ps;
    fs.release();
    if(!ps->goodInput) {
        cout << "Invalid input detected: Applicationnstopping " << endl;
        return -1;
    }


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


    cv::Mat firstImage = ps->nextImage();
    int win_width = firstImage.cols;
    int win_height = firstImage.rows;
    cout << "creating a window of : " << win_width << " x " << win_height << endl;
    window = glfwCreateWindow(win_width, win_height, "check-opengl", NULL, NULL);
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
    Calib3d calib3d(ps, &handleToKeyboardFun);

    while (!glfwWindowShouldClose(window)) {


        calib3d.processFrame( );


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
     
}
