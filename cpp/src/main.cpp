#include <iostream>
#include <opencv2/opencv.hpp>

#define SQUARE 1

using namespace cv;
using namespace std;
int main(int argc, char* argv[]) {
	//VideoCapture capture("nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)2592, height=(int)1458,format=(string)I420, framerate=(fraction)24/1 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");
	VideoCapture capture("nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)1280, height=(int)720,format=(string)I420, framerate=(fraction)24/1 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");
	if (!capture.isOpened()) {
		cout << "Cannot open the camera" << endl;
		return -1;
	}

	namedWindow("video frame BGR", WINDOW_NORMAL);
	namedWindow("Control", WINDOW_NORMAL);
	namedWindow("hsvThresholded", WINDOW_NORMAL);
	int LowH = 155;
	int HighH = 179;
	int LowS = 136;
	int HighS = 192;
	int LowV = 111;
	int HighV = 205;

	createTrackbar("LowH", "Control", &LowH, 179); //Hue (0-179)
	createTrackbar("HighH", "Control", &HighH, 179);

	createTrackbar("LowS", "Control", &LowS, 255); //Saturation (0-255)
	createTrackbar("HighS", "Control", &HighS, 255);

	createTrackbar("LowV", "Control", &LowV, 255);	//Value (0-255)
	createTrackbar("HighV", "Control", &HighV, 255);
    Mat leftCameraIntrinic = (Mat_<double>(3, 3) << 1943.5, 2.1887, 1285.8,
                                                0   , 1460.2, 733.4773,
                                                0   , 0     , 1); 
    Mat distortion = (Mat_<double>(5, 1)) << (0.0753, -0.0827, 0.0027, 0.0050, 0);

	
	while (true) {
        /* load an image from TX2 */
		Mat image_ocv;
        vector<Point2f> corner;
        vector<Point3f> object_points;
		bool bSuccess = capture.read(image_ocv);
		if (image_ocv.empty()) {
			cout << "could not load image" << endl;
			return -1; 
		}

        /* find ROI */
		Mat hsv;
        Mat image_gray;
		cvtColor(image_ocv, hsv, COLOR_BGR2HSV);
        cvtColor(image_ocv, image_gray, COLOR_BGR2GRAY); //source image to gray
        Mat hsvThresholded;
		inRange(hsv, Scalar(LowH, LowS, LowV), Scalar(HighH, HighS, HighV), hsvThresholded);
		
		//erode(hsvThresholded, hsvThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));	//fill small objects
		//dilate(hsvThresholded, hsvThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		dilate(hsvThresholded, hsvThresholded, getStructuringElement(MORPH_RECT, Size(5, 5)));	//fill small hole
		erode(hsvThresholded, hsvThresholded, getStructuringElement(MORPH_RECT, Size(5, 5)));
		vector<vector<cv::Point>> contours;
		findContours(hsvThresholded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		double maxArea = 0.0;   //filter by Area of Rectange
		cv::Rect roi;
		cv::Mat imageROI;
		int num_maxArea = 0;
        Mat image_grayROI = image_gray(roi);//ROI in Gray image
        Mat hsvROI = hsvThresholded(roi);   //ROI in Thresholded hsv_image
		if (contours.size() != 0) {
			for (int i = 0; i < contours.size(); i++) {
				if (contourArea(contours[i]) > maxArea) {
					maxArea = contourArea(contours[i]);
					num_maxArea = i;
				}
			}
			roi = cv::boundingRect(contours[num_maxArea]);
            //hsvThresholded.setTo(0);
            hsvROI.setTo(255);
			//cv::rectangle(hsvThresholded, roi, Scalar(0, 255, 255), 40);
		}
        /*
        vector<Point2f> corners;
        Size boardSize = Size(4, 11);
        bool found = findChessboardCorners(image_grayROI,
                boardSize,
                corners,
                CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE | CALIB_CB_FAST_CHECK);

        if (0 && found) {
            TermCriteria criteria = TermCriteria(
                cv::TermCriteria::EPS | cv::TermCriteria::COUNT,
                30,
                0.01);
            cornerSubPix(image_gray, corners, Size(5, 5), Size(-1, -1), criteria);
            drawChessboardCorners(image_ocv, boardSize, corners, found);
            for(int i=0; i< boardSize.width; i++)
                for (int j = 0; j < boardSize.height; j++)
                        object_points.push_back(Point3f((float)(i*SQUARE), (float)(j*SQUARE), 0.f));

            Mat rvec, tvec;
            cv::solvePnP(object_points, corners, leftCameraIntrinic, distortion, rvec, tvec);
            cv::Mat Rotation3Mat;
            Rodrigues(rvec, Rotation3Mat);  //from Vector to Mat
            cout << tvec << "   " << endl;
            cout << Rotation3Mat << endl;
        }
        */

		imshow("hsvThresholded", hsvThresholded);
		imshow("video frame BGR", image_ocv);
        char key = waitKey(5);
		if (27 == key || 'q' == key) 
			break;
	}
	return 0;
}
