#include <iostream>
#include "opencv2/core.hpp"
#include <opencv2/videoio.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/features2d.hpp>
#include "opencv2/flann.hpp"
#include "opencv2/calib3d.hpp"

using namespace std;
using namespace cv;

const char* WIN_KEYPOINTS_NAME = "Keypoints";
const char* WIN_MATCHES_NAME = "Matches";
const char* WIN_SETTINGS_NAME = "Settings";
const char* WIN_OUT_NAME = "Output";

char  input_img_paths[2][256];

Mat img_in[2], img_gray[2], img_out[2];
Mat img_sample, img_target;
Mat img_keypoints, img_matches; // intemediare images
Mat frame; // realtime webcam frame
Mat frame_gray, frame_gray32f, frame_smoothed, frame_overlay;
Mat* image_composite;

Mat descriptors_sample, descriptors_frame; // descriptors generated by chosen detector

vector<KeyPoint> keypoints_sample, keypoints_frame;

int k = 2; // k nearest neighbor
vector<vector<DMatch>> knn_matches;
vector<DMatch> matches;

/* pointers to feature extractor */
Ptr<AKAZE> akaze;
Ptr<ORB> orb;
Ptr<SIFT> sift;
Ptr<Feature2D> curr_KPDetector;

/* Keypoint detector trackbar */
const char* KPDetector_name = "KPDetector";
int KPDetector_max_value = 2;
int KPDetector_id = 0;

Ptr<Feature2D> setKPDetector(int id)
{
	switch (id) {
	case 0:
	default:
		KPDetector_name = "SIFT";
		return sift;
	case 1:
		KPDetector_name = "AKAZE";
		return akaze;
	case 2:
		KPDetector_name = "ORB";
		return orb;
	}
}

/* pointers to Descriptor Matcher */
Ptr<DescriptorMatcher> BFL1_matcher;       // - BruteForce (L1 norm)
Ptr<DescriptorMatcher> BFL2_matcher;       // - BruteForce (L2 norm)
Ptr<DescriptorMatcher> BFHamming_matcher;  // - BruteForce-Hamming
Ptr<DescriptorMatcher> FLANN_based_matcher;
Ptr<DescriptorMatcher> curr_descriptor_matcher;

/* Descriptor Matcher trackbar */
const char* descriptor_matcher_name = "Descriptor Matcher";
int descriptor_matcher_max_value = 3;
int descriptor_matcher_id = 0;

Ptr<DescriptorMatcher> setDescriptorMatcher(int id)
{
	switch (id) {
	case 0:
	default:
		descriptor_matcher_name = "FLANN based";
		return FLANN_based_matcher;
	case 1:
		descriptor_matcher_name = "BruteForce (L2 norm)";
		return BFL2_matcher;
	case 2:
		descriptor_matcher_name = "BruteForce-Hamming";
		return BFHamming_matcher;
	case 3: // KAZE
		descriptor_matcher_name = "BruteForce (L1 norm)";
		return BFL1_matcher;
	}
}

VideoCapture cap;

Scalar KPColor = Scalar::all(-1);

// signature of functions
void parseInput(int argc, char* argv[]);
void init();
void createGUI();
void update();
void callback(int value, void* userdata);
int usage(char* prgname);

int main(int argc, char* argv[])
{
	parseInput(argc, argv);
	init();

	createGUI();
	callback(KPDetector_id, 0);

	while (true)
	{
		update();

		if (waitKey(30) >= 0)
			break;
	}

	destroyAllWindows();

	return 0;
}

void parseInput(int argc, char* argv[])
{
	// Parse arguments
// this simple parser could only load images
	if (argc >= 2 * 2 + 1)
		for (int i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "-i")) {        // Input image
				if (++i > argc)
					usage(argv[0]);
				sprintf(input_img_paths[(i / 2) - 1], "%s", argv[i]);
			}
			else {
				cout << "!!! Invalid program option !!!" << argv[i] << endl;
				usage(argv[0]);
			}
		}
	else
	{
		cout << "!!! Missing argument !!!" << endl;
		usage(argv[0]);
	}

	/* store input images into the image array */
	for (int i = 0; i < 2; i++) {
		// Load image
		img_in[i] = imread(input_img_paths[i], IMREAD_UNCHANGED);
		if (img_in[i].empty()) {
			cout << "!!! Cannot read the image " << input_img_paths[i] << " !!!" << endl;
			break;
		}
		cvtColor(img_in[i], img_gray[i], COLOR_BGR2GRAY);
	}

	img_sample = img_gray[0];
	img_target = img_gray[1];
}

void init()
{
	// webcam input
	cap.open(0);

	/* Keypoint Detector pointers creation */
	akaze = AKAZE::create();
	orb = ORB::create();
	sift = SIFT::create();

	/* Descriptor Matcher pointers creation */
	BFL2_matcher = DescriptorMatcher::create("BruteForce");
	BFHamming_matcher = DescriptorMatcher::create("BruteForce-Hamming");
	FLANN_based_matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
}

void createGUI()
{
	namedWindow(WIN_SETTINGS_NAME, WINDOW_AUTOSIZE);

	/* Keypoint Detector */
	createTrackbar(KPDetector_name, WIN_SETTINGS_NAME, &KPDetector_id,
		KPDetector_max_value, (TrackbarCallback)callback);
	/* Descriptor Matcher */
	createTrackbar(descriptor_matcher_name, WIN_SETTINGS_NAME, &descriptor_matcher_id,
		descriptor_matcher_max_value, (TrackbarCallback)callback);
}

void update()
{
	// update webcam input
	cap >> frame;
	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

	/* KPDetector extraction */
	curr_KPDetector->detectAndCompute(img_sample, noArray(), keypoints_sample, descriptors_sample);
	curr_KPDetector->detectAndCompute(frame_gray, noArray(), keypoints_frame, descriptors_frame);

	drawKeypoints(img_sample, keypoints_sample, img_out[0], KPColor, DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	drawKeypoints(img_target, keypoints_frame, img_out[1], KPColor, DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	/* descriptor matching */
	/*
	FLANN_based_matcher->knnMatch(descriptors_sample, descriptors_frame, knn_matches, k);
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
	drawMatches(img_sample, keypoints_sample, frame, keypoints_frame, good_matches, img_matches, Scalar::all(-1),
		Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
		*/

		//-- Show detected (drawn) keypoints
	imshow(WIN_SETTINGS_NAME, frame);
	imshow(WIN_KEYPOINTS_NAME, img_out[0]);
	//imshow(WIN_SETTINGS_NAME, img_matches);

	/*homography*/
	 //-- Localize the object
	/*
	std::vector<Point2f> obj;
	std::vector<Point2f> scene;
	for (size_t i = 0; i < good_matches.size(); i++)
	{
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints1[good_matches[i].queryIdx].pt);
		scene.push_back(keypoints2[good_matches[i].trainIdx].pt);
	}

	Mat H = findHomography(obj, scene, RANSAC);
	warpPerspective(img_sample, frame, H, img_sample.size());
	//-- Get the corners from the image_1 ( the object to be "detected" )

//-- Get the corners from the image_1 ( the object to be "detected" )
	std::vector<Point2f> obj_corners(4);
	obj_corners[0] = Point2f(0, 0);
	obj_corners[1] = Point2f((float)img_sample.cols, 0);
	obj_corners[2] = Point2f((float)img_sample.cols, (float)img_sample.rows);
	obj_corners[3] = Point2f(0, (float)img_sample.rows);
	std::vector<Point2f> scene_corners(4);
	perspectiveTransform(obj_corners, scene_corners, H);
	//-- Draw lines between the corners (the mapped object in the scene - image_2 )
	line(img_matches, scene_corners[0] + Point2f((float)img_sample.cols, 0),
		scene_corners[1] + Point2f((float)img_sample.cols, 0), Scalar(0, 255, 0), 4);
	line(img_matches, scene_corners[1] + Point2f((float)img_sample.cols, 0),
		scene_corners[2] + Point2f((float)img_sample.cols, 0), Scalar(0, 255, 0), 4);
	line(img_matches, scene_corners[2] + Point2f((float)img_sample.cols, 0),
		scene_corners[3] + Point2f((float)img_sample.cols, 0), Scalar(0, 255, 0), 4);
	line(img_matches, scene_corners[3] + Point2f((float)img_sample.cols, 0),
		scene_corners[0] + Point2f((float)img_sample.cols, 0), Scalar(0, 255, 0), 4);
		*/

		//Mat* the_image = &img_target;

		// Concatenate source and fused images horizontally
		//hconcat(img_sample, *the_image, *image_composite);

		// Display result
		//imshow("out", *image_composite);

		//-- Show detected matches
		//imshow("Good Matches & Object detection", img_matches);

		// - Keypoint matching
	//imshow(WIN_SETTINGS_NAME, img_matches);
}

void callback(int value, void* userdata)
{
	cout << "------------------------------------------------------------------------------" << endl;
	curr_KPDetector = setKPDetector(KPDetector_id);
	cout << "> Keypoints Detector | " << KPDetector_name << endl;

	curr_descriptor_matcher = setDescriptorMatcher(descriptor_matcher_id);
	cout << "> Descriptor Matcher | " << descriptor_matcher_name << endl;
}

//-----------------------------------------------------------------------------

int usage(char* prgname)
{
	cout << "This program should load " << 2 << " images as input" << endl;
	cout << "Usage: " << prgname << " ";
	cout << "[-i {image file}]" << endl;
	exit(-1);
}

//-----------------------------------------------------------------------------