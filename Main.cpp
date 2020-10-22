#include "opencv2/core.hpp"                // OpenCV core routines
#include <opencv2/videoio.hpp>             // OpenCV video routines
#include "opencv2/highgui.hpp"             // OpenCV GUI routines
#include "opencv2/imgproc.hpp"             // OpenCV Image Processing routines
#include <iostream>
#include <opencv2/features2d.hpp>

using namespace std;
using namespace cv;                        // OpenCV classes and routines

Mat img_example;

Mat frame, frame_gray, frame_gray32f, frame_smoothed, frame_overlay;

Mat img_matches;

Mat descriptors1, descriptors2;

vector<KeyPoint> keypoints1, keypoints2;

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

	string image_path = samples::findFile("poemes.jpg");
	img_example = imread(image_path, IMREAD_COLOR);
	if (img_example.empty())
	{
		std::cout << "Could not read the image: " << image_path << std::endl;
		return 1;
	}

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
}

void update_frame()
{
	cap >> frame;

	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

	/* feature detection */
	Ptr<SIFT> siftPtr = SIFT::create();

	siftPtr->detectAndCompute(img_example, noArray(), keypoints1, descriptors1);
	siftPtr->detectAndCompute(frame, noArray(), keypoints2, descriptors2);

	/* feature description */
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);
	vector<DMatch> matches;
	matcher->match(descriptors1, descriptors2, matches);
	//-- Draw matches
	drawMatches(img_example, keypoints1, frame, keypoints2, matches, img_matches);
	//-- Show detected matches
	imshow(window_out_name, img_matches);
}