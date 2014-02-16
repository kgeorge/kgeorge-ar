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
    #include <GL/glu.h>/Users/kgeorge/Documents/ar-challenge/step5/ogl.cpp
#endif
#include "ogl.h"

const char ESC_KEY = 27;
using namespace std;
using namespace cv;

double angle(const Point &p0, const Point &p1, const Point &p2 ) {    double dx0 = p0.x - p1.x;
    double dx2 = p2.x - p1.x;
    double dy0 = p0.y - p1.y;
    double dy2 = p2.y - p1.y;
    double num = dx0 * dx2 + dy0 * dy2;
    double denom = sqrt((dx0 * dx0 + dy0  * dy0) * (dx2 * dx2 + dy2 * dy2) + 1.0e-10);
    return num/denom;
}


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
    int win_width = 800;
    int win_height = 640;
    cap >> image;
    win_width = image.cols;
    win_height = image.rows;

    const string win_name("kgeorge-ar");
    //Draw_Data draw_data;
    //init_gl(win_width, win_height, win_name, draw_data);

    OGLDraw oglDraw = OGLDraw(
        Size(win_width, win_height),
        win_name,
        &perFrameAppData);

    Scalar color_mark = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
    for (;;) {
        cap >> image;
        Mat grayImage;
        cvtColor(image, grayImage, CV_RGB2GRAY);
        Mat blurredImage;
        blur(grayImage, blurredImage, Size(5, 5));
        Mat threshImage;
        threshold(blurredImage, threshImage, 128.0, 255.0, THRESH_OTSU);
        vector<vector<Point> > contours;
        findContours(threshImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
        Scalar color(0, 255, 0);
        vector<Mat> squares;
        vector<vector<Point> > squares_pt;

        //Mat rvec, tvec;
        perFrameAppData.bValidFrame = false;
        for (auto contour : contours) {
            vector<Point> approx;
            approxPolyDP(contour, approx, arcLength(Mat(contour), true)*0.02, true);
            if( approx.size() == 4 &&
                fabs(contourArea(Mat(approx))) > 1000 &&
                isContourConvex(Mat(approx)) )
            {
                double maxCosine =  0.0;
                for(int j=2; j < 5; ++j) {
                    double cosine = angle(approx[j-2], approx[j-1], approx[j%4]);
                    maxCosine = MAX(cosine, maxCosine);
                }
                if(maxCosine < 0.3 ) {
                    Mat squareMat;
                    Mat(approx).convertTo(squareMat, CV_32FC3);
                    squares.push_back(squareMat);
                    squares_pt.push_back(approx);
                }
            }
        }
        
        
        if( squares.size() > 0) {
            perFrameAppData.bValidFrame = true;
            float side =perFrameAppData.squareSize;
            vector<Point3f> objectPoints = {
                Point3f(-side, -side, 0),
                Point3f(-side, side, 0),
                Point3f(side, side, 0),
                Point3f(side, -side, 0)
            };
            Mat rotMat(3,3, CV_32FC1);
            Mat objectPointsMat(objectPoints);
            cout << "objectPointsMat " << objectPointsMat.rows << " x " << objectPointsMat.cols;
            cout << "squares[0] " << squares[0].size() << endl;
            solvePnP(objectPointsMat, squares[0], perFrameAppData.intrinsics, perFrameAppData.distortion, perFrameAppData.rvec, perFrameAppData.tvec);
            
            //cout << "rvecs: " << perFrameAppData.rvec.rows << " x " << perFrameAppData.rvec.cols << endl;
            //cout << "tvecs: " << perFrameAppData.tvec.rows << " x " << perFrameAppData.tvec.cols << endl;
            cout << "intrinsics;" << perFrameAppData.intrinsics;
            Rodrigues( perFrameAppData.rvec, rotMat );
            
            //
            vector<Point3f> targetObjectPoints;
            targetObjectPoints.push_back(Point3f(0.0f, 0.0f, 0.0f));
            vector<Point2f> imagePoints;
            projectPoints( targetObjectPoints, perFrameAppData.rvec, perFrameAppData.tvec, perFrameAppData.intrinsics, perFrameAppData.distortion, imagePoints);
            drawContours(image, squares_pt, -1, color);
            
            Point px = squares_pt[0][0] - squares_pt[0][1];
            double lengthSegment = sqrt(px.x * px.x + px.y * px.y);
            circle( image, imagePoints[0], lengthSegment * 0.2, color_mark, -1, 8, 0);
        }
        

        color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        
        for( auto square : squares_pt ) {
            assert(square.size() == 4);
            
            for(int i=0; i < 4; ++i) {
                Point px = square[i] - square[(i+1)%4];
                double lengthSegment = sqrt(px.x * px.x + px.y * px.y);
                circle( image, square[i], lengthSegment * 0.06, color, -1, 8, 0 );
            }
        }
        //QImage qimage( (uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
        //cv::imshow(win_name, image);
        //cvShowImage("kgeorge-ar", image);
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