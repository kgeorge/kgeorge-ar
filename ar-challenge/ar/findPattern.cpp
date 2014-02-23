#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <iostream>
#include "findPattern.h"


using namespace std;
using namespace cv;



double angle2(const Point &p0, const Point &p1, const Point &p2 ) {    double dx0 = p0.x - p1.x;
    double dx2 = p2.x - p1.x;
    double dy0 = p0.y - p1.y;
    double dy2 = p2.y - p1.y;
    double num = dx0 * dx2 + dy0 * dy2;
    double denom = sqrt((dx0 * dx0 + dy0  * dy0) * (dx2 * dx2 + dy2 * dy2) + 1.0e-10);
    return num/denom;
}



void findPattern(
                 Mat &capturedImage,
                 PerFrameAppData & perFrameAppData) {
    static RNG rng1(12345), rng2(54321);
    static const Scalar color_mark( rng1.uniform(0, 255), rng1.uniform(0,255), rng1.uniform(0,255) );
    
    static const Scalar color( rng2.uniform(0, 255), rng2.uniform(0,255), rng2.uniform(0,255) );
    static const Scalar contourColor(0.0, 255.0, 0.0 );
    vector<Vec4i> hierarchy;
    
    Mat grayImage;
    cvtColor(capturedImage, grayImage, CV_RGB2GRAY);
    Mat blurredImage;
    blur(grayImage, blurredImage, Size(5, 5));
    Mat threshImage;
    threshold(blurredImage, threshImage, 128.0, 255.0, THRESH_OTSU);
    vector<vector<Point> > contours;
    findContours(threshImage, contours,hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
    vector<Mat> squares;
    vector<vector<Point> > squares_pt;
    
    //Mat rvec, tvec;
    vector<int> indicesOfSquareContours;
    perFrameAppData.bValidFrame = false;
    for (int i=0; i< contours.size(); ++i) {
        auto contour = contours[i];
        vector<Point> approx;
        approxPolyDP(contour, approx, arcLength(Mat(contour), true)*0.02, true);
        if( approx.size() == 4 &&
           fabs(contourArea(Mat(approx))) > 1000 &&
           isContourConvex(Mat(approx)) )
        {
            double maxCosine =  0.0;
            for(int j=2; j < 5; ++j) {
                double cosine = angle2(approx[j-2], approx[j-1], approx[j%4]);
                maxCosine = MAX(cosine, maxCosine);
            }
            if(maxCosine < 0.3 ) {
                Mat squareMat;
                Mat(approx).convertTo(squareMat, CV_32FC3);
                squares.push_back(squareMat);
                squares_pt.push_back(approx);
                indicesOfSquareContours.push_back(i);
            }
        }
    }
    bool isCounterClockwise = false;
    vector<vector<Point>> adjustedContours;
    adjustedContours.push_back( vector<Point>() );
    vector<Point> &adjustedContour = adjustedContours[0];
    Point smallSquareCenter;
    double high_level_contour_area, low_level_contour_area, ratio;
    if(indicesOfSquareContours.size() == 2) {
        for(int i=0; i < indicesOfSquareContours.size(); ++i) {
            int idx = indicesOfSquareContours[i];
            Vec4i h = hierarchy[idx];
            if( h[2] >= 0 ) {
                cout << "next: " << h[0] << ", previous: " <<  h[1] << ", child: " << h[2] << ", parent: " <<  h[3] << endl;       \
                high_level_contour_area = contourArea(contours[idx], true);
                low_level_contour_area = contourArea(contours[h[2]], true);
                ratio = fabs(high_level_contour_area)/fabs(low_level_contour_area  + 0.0001);
                if( ratio < 60.0 && ratio > 23.0) {
                    Mat smallSquareContour(squares_pt[i]);
                    Moments mmnts = moments(contours[h[2]],false);
                    smallSquareCenter.x = (mmnts.m10 / mmnts.m00);
                    smallSquareCenter.y = (mmnts.m01 / mmnts.m00);
                    double min_diff = numeric_limits<double>::max(), cur_diff;
                    int min_idx = 0;
                    for (int j=0;  j <  (int)(squares_pt[i].size()); ++j) {
                        Point d2 = squares_pt[i][j] - smallSquareCenter;
                        cur_diff = norm(d2);
                        min_idx =   (min_diff > cur_diff) ? j : min_idx;
                        min_diff = (min_diff > cur_diff) ? cur_diff : min_diff;
                        cout << "~~~~: j=" << j << "  cur_diff: " << cur_diff << "  min_diff: " << min_diff << " min_idx: " << min_idx << " : d2 " << d2 << endl;
                        cout << " amsllSq center: " << smallSquareCenter << endl;
                    }
                    for(int j =0; j < (int)(squares_pt[i].size()); ++j) {
                        adjustedContour.push_back(squares_pt[i][(j + min_idx) % squares_pt[i].size()]);
                    }
                    CV_Assert( adjustedContour.size() > 2);
                    Point &p0 = adjustedContour[0];
                    Point &p1 = adjustedContour[1];
                    isCounterClockwise = (p0.x *  p1.y - p0.y * p1.x) < 0 ?  true: false;
                    cout << "high " << high_level_contour_area << " low " << low_level_contour_area << endl;
                    cout << "isCouterClockwise: " << isCounterClockwise  << " min_idx: "  << min_idx;
                    break;
                }
            }
        }
    }
    
    CV_Assert(indicesOfSquareContours.size() == squares_pt.size());
    
    if( adjustedContour.size() > 0) {
        perFrameAppData.bValidFrame = true;
        float side =perFrameAppData.squareSize;
        vector<Point3f> objectPoints;
        if(isCounterClockwise) {
            
            objectPoints.push_back( Point3f(-side, -side, 0) );
            objectPoints.push_back( Point3f(side, -side, 0) );
            objectPoints.push_back( Point3f(side, side, 0) );
            objectPoints.push_back( Point3f(-side, side, 0) );
            
        } else {
            
            objectPoints.push_back( Point3f(-side, -side, 0) );
            objectPoints.push_back( Point3f(-side, side, 0) );
            objectPoints.push_back( Point3f(side, side, 0) );
            objectPoints.push_back( Point3f(side, -side, 0) );
            
        };
        Mat rotMat(3,3, CV_32FC1);
        Mat objectPointsMat(objectPoints);
        cout << "objectPointsMat " << objectPointsMat.rows << " x " << objectPointsMat.cols;
        cout << "squares[0] " << squares[0].size() << endl;
        Mat sq(adjustedContour);
        Mat sq2;
        sq.convertTo(sq2, CV_32FC2);
        solvePnP(objectPointsMat, sq2, perFrameAppData.intrinsics, perFrameAppData.distortion, perFrameAppData.rvec, perFrameAppData.tvec);
        
        //cout << "rvecs: " << perFrameAppData.rvec.rows << " x " << perFrameAppData.rvec.cols << endl;
        //cout << "tvecs: " << perFrameAppData.tvec.rows << " x " << perFrameAppData.tvec.cols << endl;
        cout << "intrinsics;" << perFrameAppData.intrinsics;
        Rodrigues( perFrameAppData.rvec, rotMat );
        
        //
        vector<Point3f> targetObjectPoints;
        targetObjectPoints.push_back(Point3f(0.0f, 0.0f, 0.0f));
        vector<Point2f> imagePoints;
        projectPoints( targetObjectPoints, perFrameAppData.rvec, perFrameAppData.tvec, perFrameAppData.intrinsics, perFrameAppData.distortion, imagePoints);
        drawContours(capturedImage, adjustedContours, -1, color);
        
        Point px = adjustedContour[0] - adjustedContour[1];
        double lengthSegment = sqrt(px.x * px.x + px.y * px.y);
        circle( capturedImage, imagePoints[0], lengthSegment * 0.2, color_mark, -1, 8, 0);
    }
    for( auto square : squares_pt  ) {
        assert(square.size() == 4);
        for(int i=0; i < 4; ++i) {
            Point px = square[i] - square[(i+1)%4];
            double lengthSegment = sqrt(px.x * px.x + px.y * px.y);
            circle( capturedImage, square[i], lengthSegment * 0.06, color, -1, 8, 0 );
        }
    }

    
    
    
    
    
    
    
}