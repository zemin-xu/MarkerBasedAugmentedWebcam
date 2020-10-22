#include "opencv2/core.hpp"                // OpenCV core routines
#include <opencv2/videoio.hpp>             // OpenCV video routines
#include "opencv2/highgui.hpp"             // OpenCV GUI routines
#include "opencv2/imgproc.hpp"             // OpenCV Image Processing routines
#include <iostream>
#include <opencv2/features2d.hpp>

using namespace std;
using namespace cv;                        // OpenCV classes and routines

Mat frame, frame_gray, frame_gray32f, frame_smoothed, frame_overlay;
Mat* the_frame;

vector<KeyPoint> keypoints;

VideoCapture cap;
bool isGrayCamera = false;

Scalar KPColor = Scalar::all(-1);
char window_out_name[256] = "output";

// signature of functions
void create_GUI();

void update_frame();
int main(int argc, char* argv[])
{
	//create_GUI();

	cap.open(0);
	cap >> frame;
	if (frame.channels() == 1)
		isGrayCamera = true;

	while (true)
	{
		update_frame();
		// Listen to next event - Exit if key pressed
		if (waitKey(30) >= 0)
			break;
	}

	destroyAllWindows();
	return 0;
}

void create_GUI()
{
	/*
	Mat img1 = imread("box.png", IMREAD_GRAYSCALE);
	Mat img2 = imread("box_in_scene.png", IMREAD_GRAYSCALE);

	Mat image_out = img1;
	Mat descriptor;

	Ptr<ORB> orb = ORB::create(1);
	orb->detectAndCompute(img1, Mat(), keypoints, descriptor);

	// Overlay keypoints onto original images
	drawKeypoints(img1, keypoints, image_out,
		KPColor, DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	//	imshow(window_out_name, image_out);

	//-- Step 1: Detect the keypoints using SURF Detector
	Ptr<SIFT> detector = SIFT::create();
	vector<KeyPoint> kps;
	Mat img_gray;
	cvtColor(img2, img_gray, COLOR_BGR2GRAY);
	detector->detect(img_gray, kps);
	//-- Draw keypoints
	drawKeypoints(img2, kps, image_out);
	//-- Show detected (drawn) keypoints
	imshow("SIFT Keypoints", image_out);
	*/
}

void update_frame()
{
	vector<KeyPoint> keypoints1, keypoints2;

	cap >> frame;

	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

	/* feature detection */
	Ptr<SIFT> siftPtr = SIFT::create();
	//siftPtr->detect(frame_gray, keypoints);
	//drawKeypoints(frame, keypoints, frame);

	//-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
	Mat descriptors1, descriptors2;
	siftPtr->detectAndCompute(frame, noArray(), keypoints1, descriptors1);
	siftPtr->detectAndCompute(frame, noArray(), keypoints2, descriptors2);

	/* feature description */
	//-- Step 2: Matching descriptor vectors with a brute force matcher
	// Since SURF is a floating-point descriptor NORM_L2 is used
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);
	std::vector< DMatch > matches;
	matcher->match(descriptors1, descriptors2, matches);
	//-- Draw matches
	Mat img_matches;
	drawMatches(frame, keypoints1, frame, keypoints2, matches, img_matches);
	//-- Show detected matches
	imshow("Matches", img_matches);

	//imshow(window_out_name, frame);
}