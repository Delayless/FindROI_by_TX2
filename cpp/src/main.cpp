#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
int main(int argc, char* argv[]) {
	VideoCapture capture("nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)1280, height=(int)720,format=(string)I420, framerate=(fraction)24/1 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");
	if (!capture.isOpened()) {
		cout << "Cannot open the camera" << endl;
		return -1;
	}

	namedWindow("juju", WINDOW_NORMAL);
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
	
	while (true) {

		Mat src;
		cin.clear();
		cin.sync();   //»òÕßÓÃcin.ignore();
		bool bSuccess = capture.read(src);   
		if (src.empty()) {
			cout << "could not load image" << endl; return -1; 
		}

		Mat hsv;
		cvtColor(src, hsv, COLOR_BGR2HSV);

		Mat hsvThresholded;
		inRange(hsv, Scalar(LowH, LowS, LowV), Scalar(HighH, HighS, HighV), hsvThresholded);
		
		//erode(hsvThresholded, hsvThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));	//fill small objects
		//dilate(hsvThresholded, hsvThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		dilate(hsvThresholded, hsvThresholded, getStructuringElement(MORPH_RECT, Size(5, 5)));	//fill small hole
		erode(hsvThresholded, hsvThresholded, getStructuringElement(MORPH_RECT, Size(3, 3)));
		vector<vector<cv::Point>> contours;
		findContours(hsvThresholded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		double maxArea = 0.0;
		cv::Rect roi;
		cv::Mat imageROI;
		int num_maxArea = 0;
		if (contours.size() != 0) {
			for (int i = 0; i < contours.size(); i++) {
				if (contourArea(contours[i]) > maxArea) {
					maxArea = contourArea(contours[i]);
					num_maxArea = i;
				}
			}
			roi = cv::boundingRect(contours[num_maxArea]);
			imageROI = hsvThresholded(roi);
			imageROI.setTo(255);
			//cv::rectangle(hsvThresholded, roi, Scalar(0, 255, 255), 40);
			
		}

		imshow("hsvThresholded", hsvThresholded);
		imshow("juju", src);
		if (27 == waitKey(5))
			break;
	}
	return 0;
}
