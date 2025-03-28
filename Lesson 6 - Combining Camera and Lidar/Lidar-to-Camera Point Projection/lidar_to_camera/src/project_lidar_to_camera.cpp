#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "structIO.hpp"

using namespace std;

// Set calibration values exactly as the ones uploaded by people from KITTI dataset
void loadCalibrationData(cv::Mat &P_rect_00, cv::Mat &R_rect_00, cv::Mat &RT) {
    RT.at<double>(0,0) = 7.533745e-03; RT.at<double>(0,1) = -9.999714e-01; RT.at<double>(0,2) = -6.166020e-04; RT.at<double>(0,3) = -4.069766e-03;
    RT.at<double>(1,0) = 1.480249e-02; RT.at<double>(1,1) = 7.280733e-04; RT.at<double>(1,2) = -9.998902e-01; RT.at<double>(1,3) = -7.631618e-02;
    RT.at<double>(2,0) = 9.998621e-01; RT.at<double>(2,1) = 7.523790e-03; RT.at<double>(2,2) = 1.480755e-02; RT.at<double>(2,3) = -2.717806e-01;
    RT.at<double>(3,0) = 0.0; RT.at<double>(3,1) = 0.0; RT.at<double>(3,2) = 0.0; RT.at<double>(3,3) = 1.0;
    
    R_rect_00.at<double>(0,0) = 9.999239e-01; R_rect_00.at<double>(0,1) = 9.837760e-03; R_rect_00.at<double>(0,2) = -7.445048e-03; R_rect_00.at<double>(0,3) = 0.0;
    R_rect_00.at<double>(1,0) = -9.869795e-03; R_rect_00.at<double>(1,1) = 9.999421e-01; R_rect_00.at<double>(1,2) = -4.278459e-03; R_rect_00.at<double>(1,3) = 0.0;
    R_rect_00.at<double>(2,0) = 7.402527e-03; R_rect_00.at<double>(2,1) = 4.351614e-03; R_rect_00.at<double>(2,2) = 9.999631e-01; R_rect_00.at<double>(2,3) = 0.0;
    R_rect_00.at<double>(3,0) = 0; R_rect_00.at<double>(3,1) = 0; R_rect_00.at<double>(3,2) = 0; R_rect_00.at<double>(3,3) = 1;
    
    P_rect_00.at<double>(0,0) = 7.215377e+02; P_rect_00.at<double>(0,1) = 0.000000e+00; P_rect_00.at<double>(0,2) = 6.095593e+02; P_rect_00.at<double>(0,3) = 0.000000e+00;
    P_rect_00.at<double>(1,0) = 0.000000e+00; P_rect_00.at<double>(1,1) = 7.215377e+02; P_rect_00.at<double>(1,2) = 1.728540e+02; P_rect_00.at<double>(1,3) = 0.000000e+00;
    P_rect_00.at<double>(2,0) = 0.000000e+00; P_rect_00.at<double>(2,1) = 0.000000e+00; P_rect_00.at<double>(2,2) = 1.000000e+00; P_rect_00.at<double>(2,3) = 0.000000e+00;

}

void projectLidarToCamera2()
{
    // load image from file
    cv::Mat img = cv::imread("../images/0000000000.png");

    // load Lidar points from file
    std::vector<LidarPoint> lidarPoints;
    readLidarPts("../dat/C51_LidarPts_0000.dat", lidarPoints);

    // store calibration data in OpenCV matrices
    cv::Mat P_rect_00(3,4,cv::DataType<double>::type); // 3x4 projection matrix after rectification
    cv::Mat R_rect_00(4,4,cv::DataType<double>::type); // 3x3 rectifying rotation to make image planes co-planar
    cv::Mat RT(4,4,cv::DataType<double>::type); // rotation matrix and translation vector
    loadCalibrationData(P_rect_00, R_rect_00, RT);
    
    cv::Mat visImg = img.clone(); 
    cv::Mat overlay = visImg.clone(); // making trasnaprent so we can overaly the points

    cv::Mat X(4,1,cv::DataType<double>::type);
    cv::Mat Y(3,1,cv::DataType<double>::type);

    //* Run over all lidar points, and map them to xy coordinate
    for(auto it=lidarPoints.begin(); it!=lidarPoints.end(); ++it) {
        // 1. Convert current Lidar point into homogeneous coordinates and store it in the 4D variable X.
        X.at<double>(0, 0) = it->x;
        X.at<double>(1, 0) = it->y;
        X.at<double>(2, 0) = it->z;
        X.at<double>(3, 0) = 1;

        // 2. Then, apply the projection equation using the extrinsic and intrinsic formulas to map X onto the image plane of the camera. 
        // Store the result in Y.
        // X point in 3D space (but homogeneous coordinates to make easy calculations), RT (excentric calibration, rotation + calibration), R_rect (rectify cameras so two cameras match but we are not using this because dont have stereo), P_rect (intrinsic calibration, which is lens distortion, principal point, focla length)
        Y = P_rect_00 * R_rect_00 * RT * X; // easy to concatanate because in homogeneous coordinates. Get Y(3,1)

        // 3. Once this is done, transform Y back into Euclidean coordinates and store the result in the variable pt, to get into pixel coordinates.
        Y /= Y.at<double>(2,0);
        cv::Point pt(Y.at<double>(0,0), Y.at<double>(1,0));

        //* Filter points that are within distance only
        float maxX = 25.0, maxY = 6.0, minZ = -1.4; 
        if(it->x > maxX || it->x < 0.0 || abs(it->y) > maxY || it->z < minZ || it->r<0.01 )
            continue; // skip to next point when not within boundaries
        
        //* Color the current point to red if close, green if far. Circle detailing the lidar
        float val = it->x;
        float maxVal = 20.0;
        int red = min(255, (int)(255 * abs((val - maxVal) / maxVal)));
        int green = min(255, (int)(255 * (1 - abs((val - maxVal) / maxVal))));
        cv::circle(overlay, pt, 5, cv::Scalar(0, green, red), -1);
    }

    float opacity = 0.6; // opacity of image to show
    cv::addWeighted(overlay, opacity, visImg, 1 - opacity, 0, visImg);
    
    string windowName = "LiDAR data on image overlay";
    cv::namedWindow( windowName, 3 );
    cv::imshow( windowName, visImg );
    cv::waitKey(0); // wait for key to be pressed
}

int main()
{
    projectLidarToCamera2();
}