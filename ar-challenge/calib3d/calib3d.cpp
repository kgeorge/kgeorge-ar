#include <iostream>
#include <sstream>
#include "calib3d.h"
#include "settings.h"

using namespace cv;
using namespace std;

const char ESC_KEY = 27;
const cv::Scalar RED(0,0,255);
const cv::Scalar GREEN(0,255,0);


static void calcBoardCornerPositions(
                                     const Size &boardSize,
                                     float squareSize,
                                     Settings::Pattern patternType,
                                     vector<Point3f>& corners
                                     )
{
    corners.clear();
    float halfW = boardSize.width * squareSize;
    float halfH = boardSize.height * squareSize;
    switch(patternType) {
            case Settings::CHESSBOARD:
            case Settings::CIRCLES_GRID:
                for( int i=0; i < boardSize.height; ++i) {
                    for( int j=0; j < boardSize.width; ++j) {
                        corners.push_back ( Point3f((j*squareSize - halfW), i*squareSize - halfH, 0.0) );
                    }
                }
                break;
            case Settings::ASYMMETRIC_CIRCLES_GRID:
            default:
            assert(false);
                break;
    }
}

static bool validate_output_sizes(
                                  const vector<Mat> &rvecs,
                                  const vector<Mat> &tvecs,
                                  const vector<vector<Point3f>> &objectPoints,
                                  const vector<vector<Point2f>> &imagePoints,
                                  const vector<float> &reProjectionErrors
                                  ) {
    int n_rvecs = rvecs.size();
    int n_tvecs = tvecs.size();
    int n_views_from_object_points = objectPoints.size();
    int n_views_from_images = imagePoints.size();
    int n_reProjectionErrors = reProjectionErrors.size();
    
    return
    n_rvecs == n_tvecs &&
    n_rvecs == n_views_from_object_points &&
    n_rvecs == n_views_from_images &&
    n_rvecs == n_reProjectionErrors;
    
}

static double computeReprjectionError(
        const Mat & cameraMatrix,
        const Mat & distCoeffs,
        const vector<Mat> &rvecs,
        const vector<Mat> &tvecs,
        const vector<vector<Point3f>> objectPoints,
        const vector<vector<Point2f>> &imagePoints,
        vector<float> &perViewErrors
        ) {
    vector<Point2f> imageReProjection;
    Mat jacobian;
    double err;
    double totalErr = 0;
    int nTotalViews = objectPoints.size();
    int nTotalPoints =0;
    int nPointsInThisView =0;
    perViewErrors.resize(nTotalViews);
    for(int i=0; i < nTotalViews; ++i) {
        nPointsInThisView =  imagePoints[i].size();        assert(nPointsInThisView == objectPoints[i].size());
        projectPoints(
                      objectPoints[i],
                      rvecs[i],
                      tvecs[i],
                      cameraMatrix,
                      distCoeffs,
                      imageReProjection,
                      jacobian);
        
        assert(nPointsInThisView ==  imageReProjection.size());
        err = norm(Mat(imageReProjection), Mat(imagePoints[i]), CV_L2);
        perViewErrors[i] = (err * err) / nPointsInThisView;
        totalErr += err * err;
        nTotalPoints += nPointsInThisView;
    }
    return std::sqrt(totalErr/nTotalPoints);
}

static bool runCalibration(
                           const Settings& s,
                           const Size& imageSize,
                           const vector<vector<Point2f> > &imagePoints,
                           Mat& cameraMatrix,
                           Mat& distCoeffs,
                           vector<Mat> &rvecs,
                           vector<Mat> &tvecs,
                           vector<float> &perViewErrors,
                           double &averageReprojectionError
                           ) {
    
    
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = 1.0;
    distCoeffs = Mat::zeros(8, 1, CV_64F);
    vector<vector<Point3f> > objectPoints(1);
    calcBoardCornerPositions(s.boardSize, s.squareSize, s.calibrationPattern, objectPoints[0]);
    objectPoints.resize(imagePoints.size(),objectPoints[0]);
    double rms = calibrateCamera(
        objectPoints,
        imagePoints,
        imageSize,
        cameraMatrix,
        distCoeffs,
        rvecs,
        tvecs,
        s.flag | CV_CALIB_FIX_K4|CV_CALIB_FIX_K5
                                 );
    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;
    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);
    averageReprojectionError = computeReprjectionError(
                            cameraMatrix,
                            distCoeffs,
                            rvecs,
                            tvecs,
                            objectPoints,
                            imagePoints,
                            perViewErrors
    );
    ok = ok && validate_output_sizes(
        rvecs,
        tvecs,
        objectPoints,
        imagePoints,
        perViewErrors
                          );
    return ok;
    
}

// Print camera parameters to the output file
static void saveCameraParams(
                             const Settings& s,
                             const Mat &cameraMatrix,
                             const Mat &distCoeffs,
                             const vector<vector<Point2f>> &imagePoints,
                             const vector<Mat> &rvecs,
                             const vector<Mat> &tvecs,
                             const vector<float> &perViewErrors,
                             const Size &imageSize,
                             double averageReprojectionError
                             )
{
    FileStorage fs( s.outputFileName, FileStorage::WRITE );
    
    time_t t;
    char temp_char_buf[1024];
    time(&t);
    struct tm tm_1 = *(localtime(&t));
    strftime(temp_char_buf, sizeof(temp_char_buf)-1, "%c", &tm_1);
    fs << "calibration_Time" << temp_char_buf;
    if(!rvecs.empty()) {
        fs << "nrOfFrames" << static_cast<int>(rvecs.size());
    }
    fs << "image_Width" << imageSize.width;
    fs << "image_Height" << imageSize.height;
    fs << "board_Width" << s.boardSize.width;
    fs << "board_Height" << s.boardSize.height;
    fs << "square_Size" << s.squareSize;
    if(s.flag & CV_CALIB_FIX_ASPECT_RATIO ) {
        fs << "FixAspectRatio" << s.aspectRatio;
    }
    if(s.flag) {
        sprintf(temp_char_buf, "flags: %s%s%s%s",
            ( s.flag & CV_CALIB_USE_INTRINSIC_GUESS ) ? " +use_intrinsic_guess" : "",
            ( s.flag & CV_CALIB_FIX_ASPECT_RATIO ) ? " +fix_aspectRatio" : "",
            ( s.flag & CV_CALIB_FIX_PRINCIPAL_POINT ) ? " +fix_principal_point" : "",
            ( s.flag & CV_CALIB_ZERO_TANGENT_DIST ) ? " +zero_tangent_dist" : "");
        cvWriteComment(*fs, temp_char_buf, 0);
    }
    fs << "flagValue" << s.flag;
    fs << "Camera_Matrix" << cameraMatrix;
    fs << "Distortion_Coefficients" << distCoeffs;
    fs << "Avg_Reprojection_Error" << averageReprojectionError;
    if(!perViewErrors.empty()) {
        fs << "Per_View_Reprojection_Errors" << Mat(perViewErrors);
    }
    if(!rvecs.empty() && !tvecs.empty()) {
        CV_Assert(rvecs[0].type() == tvecs[0].type());
        Mat bigMat = Mat(
                         static_cast<int>(rvecs.size()),
                         6,
                         rvecs[0].type()
                         );
        for(int i=0; i < static_cast<int>(rvecs.size()); ++i) {
            Mat r = bigMat(Range(i, i+1), Range(0,3));
            Mat t = bigMat(Range(i, i+1), Range(3, 6));
            CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
            CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
            r = rvecs[i].t();
            t = tvecs[i].t();
        }
        cvWriteComment(*fs, " rvecs, tvecs := a set of 6 tuples, rotation in Rodriques followed by translation ", 0);
        fs << "Extrinsic_Parameters" << bigMat;
        
        if(!imagePoints.empty()) {
            Mat imagePointBigMat(
                static_cast<int>(imagePoints.size()),
                static_cast<int>(imagePoints[0].size()),
                CV_32FC2
            );
            for(int i=0; i < static_cast<int>(imagePoints.size()); ++i)  {
                Mat r = imagePointBigMat.row(i).reshape(2, imagePointBigMat.cols);
                Mat imgPoints_i(imagePoints[i]);
                imgPoints_i.copyTo(r);
                int j=0;
                j = j+1;
            }
            fs << "Image_points" << imagePointBigMat;
        }
    }
}
static bool runCalibrationAndSave(
                           const Settings &s,
                           const Size &imageSize,
                           const vector<vector<Point2f>> &imagePoints,
                           Mat & cameraMatrix,
                           Mat & distCoeffs) {
    vector<Mat> rvecs, tvecs;
    double averageReprojectionError = 0.0;
    vector<float> perViewErrors;
    bool runCalibrationOk = runCalibration(
                                            s,
                                            imageSize,
                                            imagePoints,
                                            cameraMatrix,
                                            distCoeffs,
                                            rvecs,
                                            tvecs,
                                            perViewErrors,
                                            averageReprojectionError);
    if(runCalibrationOk) {
        cout <<
            "calibation Succeeded, reprojection error:  " << averageReprojectionError <<
            " numImagePoints: " << imagePoints[0].size() <<
            " numViews: " << imagePoints.size() <<
        endl;
        saveCameraParams(
                         s,
                         cameraMatrix,
                         distCoeffs,
                         imagePoints,
                         rvecs,
                         tvecs,
                         perViewErrors,
                         imageSize,
                         averageReprojectionError
                         );
        
    } else {
        cout <<
        "calibration failed!" << endl;
    }

    return runCalibrationOk;
}


Calib3d::Calib3d(
        Settings *ps,
        std::vector< std::function<void(char)>> *pHandleToKeyboardFun):
        ps(ps),
        pHandleToKeyboardFun(pHandleToKeyboardFun) {
        mode = ps->inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
        boundKeyboardFunBody = std::bind(keyBoardFun, this, _1 );
        pHandleToKeyboardFun->push_back(boundKeyboardFunBody);
    }
Calib3d::~Calib3d() {
        delete ps;
        pHandleToKeyboardFun->clear();
    }

void Calib3d::processFrame() {
    Mat view = ps->nextImage();
    if( mode == CAPTURING && imagePoints.size() >= (unsigned)ps->nrFrames )
    {
        if( runCalibrationAndSave(*ps, view.size(), imagePoints, cameraMatrix, distCoeffs))
            mode = CALIBRATED;
        else
            mode = DETECTION;
    }
    imageSize = view.size();  // Format input image.
    
    if( ps->flipVertical )    flip( view, view, 0 );
    vector<Point2f> pointBuf;
    bool found;
    switch( ps->calibrationPattern ) // Find feature points on the input format
    {
        case Settings::CHESSBOARD:
            found = findChessboardCorners( view, ps->boardSize, pointBuf,
                                          CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
            break;
        case Settings::CIRCLES_GRID:
            found = findCirclesGrid( view, ps->boardSize, pointBuf );
            break;
        case Settings::ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid( view, ps->boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID );
            break;
        default:
            found = false;
            break;
    }
    if(found) {
        if( ps->calibrationPattern == Settings::CHESSBOARD) {
            Mat viewGray;
            cvtColor( view, viewGray, CV_BGR2GRAY );
            cornerSubPix(
                         viewGray,
                         pointBuf,
                         Size(11,11),
                         Size(-1,-1),
                         TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 )
                         );
            
            
        }
        if( mode == CAPTURING &&
           (!ps->inputCapture.isOpened()||  clock() - prevTimestamp > ps->delay*1e-3*CLOCKS_PER_SEC )
           ) {
            imagePoints.push_back(pointBuf);
            prevTimestamp = clock();
        }
        drawChessboardCorners( view, ps->boardSize, pointBuf, found );
    }
    
    string msg = (mode == CAPTURING) ? "100/100" : mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
    int baseLine = 0;
    Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
    Point textOrigin(view.cols - 2*textSize.width - 10, view.rows - 2*baseLine - 10);
    if( mode == CAPTURING )
    {
        msg = format( "%d/%d", (int)imagePoints.size(), ps->nrFrames );
    }
    background.processFrame(view);
    background.draw();
    
}

void Calib3d::handleKey(char keyCode) {
        if( ps->inputCapture.isOpened() && (keyCode == 'g' || keyCode == 'G') )
        {
            mode = CAPTURING;
            imagePoints.clear();
        }
}