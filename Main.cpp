#include "opencv2/core.hpp"                // OpenCV core routines
#include <opencv2/videoio.hpp>             // OpenCV video routines
#include "opencv2/highgui.hpp"             // OpenCV GUI routines
#include "opencv2/imgproc.hpp"             // OpenCV Image Processing routines
#include <iostream>
#include <opencv2/features2d.hpp>
#include "opencv2/flann.hpp"
#include "opencv2/calib3d.hpp"

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
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
	std::vector< std::vector<DMatch> > knn_matches;
	matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);
	//-- Filter matches using the Lowe's ratio test
	const float ratio_threshold = 0.7f;
	std::vector<DMatch> good_matches;
	for (size_t i = 0; i < knn_matches.size(); i++)
	{
		if (knn_matches[i][0].distance < ratio_threshold * knn_matches[i][1].distance)
		{
			good_matches.push_back(knn_matches[i][0]);
		}
	}
	//-- Draw matches
	Mat img_matches;
	drawMatches(img_example, keypoints1, frame, keypoints2, good_matches, img_matches, Scalar::all(-1),
		Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	/*homography*/
	 //-- Localize the object
	std::vector<Point2f> obj;
	std::vector<Point2f> scene;
	for (size_t i = 0; i < good_matches.size(); i++)
	{
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints1[good_matches[i].queryIdx].pt);
		scene.push_back(keypoints2[good_matches[i].trainIdx].pt);
	}
	Mat H = findHomography(obj, scene, RANSAC);
	//-- Get the corners from the image_1 ( the object to be "detected" )
	std::vector<Point2f> obj_corners(4);
	obj_corners[0] = Point2f(0, 0);
	obj_corners[1] = Point2f((float)img_example.cols, 0);
	obj_corners[2] = Point2f((float)img_example.cols, (float)img_example.rows);
	obj_corners[3] = Point2f(0, (float)img_example.rows);
	std::vector<Point2f> scene_corners(4);
	perspectiveTransform(obj_corners, scene_corners, H);
	//-- Draw lines between the corners (the mapped object in the scene - image_2 )
	line(img_matches, scene_corners[0] + Point2f((float)img_example.cols, 0),
		scene_corners[1] + Point2f((float)img_example.cols, 0), Scalar(0, 255, 0), 4);
	line(img_matches, scene_corners[1] + Point2f((float)img_example.cols, 0),
		scene_corners[2] + Point2f((float)img_example.cols, 0), Scalar(0, 255, 0), 4);
	line(img_matches, scene_corners[2] + Point2f((float)img_example.cols, 0),
		scene_corners[3] + Point2f((float)img_example.cols, 0), Scalar(0, 255, 0), 4);
	line(img_matches, scene_corners[3] + Point2f((float)img_example.cols, 0),
		scene_corners[0] + Point2f((float)img_example.cols, 0), Scalar(0, 255, 0), 4);

	imshow(window_out_name, img_matches);
}