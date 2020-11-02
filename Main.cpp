#include <iostream>
#include <time.h>
#include "opencv2/core.hpp"
#include <opencv2/videoio.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/features2d.hpp>
#include "opencv2/flann.hpp"
#include "opencv2/calib3d.hpp"
#include "ImageUtilities.h"

#include <opencv2/xfeatures2d.hpp>

using namespace std;
using namespace cv;

vector<float> percentage;
float average;

const char* WIN_KEYPOINTS_NAME = "Keypoints";
const char* WIN_MATCHES_NAME = "Matches";
const char* WIN_SETTINGS_NAME = "Settings";
const char* WIN_AUGMENTATION_NAME = "Augmentation";

bool TEST_MODE = false; // whether all the parameters be shown

char  input_img_paths[2][256]; // path for input images

Mat img_in[2];
Mat img_sample; // gray version of sample image to match in frame
Mat img_target; // augmented image onto matching instance
Mat img_keypoints, img_matches; // images for each intemediare step
Mat frame, frame_gray; // realtime webcam frame and gray image
Mat img_homography; // perspective transformation of target image
Mat img_overlay; // overlay image of homography with frame
Mat frame_augmented; // final frame with augmented image, with effect of opacity

Mat descriptors_sample, descriptors_frame; // descriptors generated by chosen detector

vector<KeyPoint> keypoints_sample, keypoints_frame;

int k = 2; // k nearest neighbor
vector<vector<DMatch>> knn_matches;
vector<DMatch> matches;

vector<Point2f> points_sample;
vector<Point2f> points_frame;

vector<Point2f> corners_sample(4);
vector<Point2f> corners_frame(4);
VideoCapture cap;

/* pointers to feature extractor */
Ptr<AKAZE> akaze;
Ptr<ORB> orb;
Ptr<SIFT> sift;
Ptr<BRISK> brisk;
Ptr<xfeatures2d::SURF> surf;

Ptr<Feature2D> curr_KPDetector;

/* Keypoint detector trackbar */
const char* KPDetector_name = "KPDetector";
int KPDetector_max_value = 4;
int KPDetector_id = 4;

Ptr<Feature2D> setKPDetector(int id)
{
	switch (id) {
	case 0:
	default:
		KPDetector_name = "SIFT";
		return sift;
	case 1:
		KPDetector_name = "SURF";
		return surf;
	case 2:
		KPDetector_name = "ORB";
		return orb;
	case 3:
		KPDetector_name = "AKAZE";
		return akaze;
	case 4:
		KPDetector_name = "BRISK";
		return brisk;
	}
}

/* pointers to Descriptor Matcher */
Ptr<BFMatcher> BFL1_matcher;       // - BruteForce (L1 norm)
Ptr<BFMatcher> BFL2_matcher;       // - BruteForce (L2 norm)
Ptr<BFMatcher> BFHamming_matcher;  // - BruteForce-Hamming
Ptr<FlannBasedMatcher> FLANN_based_matcher; // - FLANN based
Ptr<DescriptorMatcher> curr_descriptor_matcher;

/* Descriptor Matcher trackbar */
const char* descriptor_matcher_name = "Descriptor Matcher";
int descriptor_matcher_max_value = 3;
int descriptor_matcher_id = 3;

Ptr<DescriptorMatcher> setDescriptorMatcher(int id)
{
	switch (id) {
	case 0:
	default:
		descriptor_matcher_name = "FLANN based"; // suitable for SURF, SIFT
		return FLANN_based_matcher;
	case 1:
		descriptor_matcher_name = "BruteForce (L1 norm)";
		return BFL1_matcher;
	case 2:
		descriptor_matcher_name = "BruteForce (L2 norm)";
		return BFL2_matcher;
	case 3:
		descriptor_matcher_name = "BruteForce-Hamming"; // suitable for ORB, AKAZE, BRISK
		return BFHamming_matcher;
	}
}

/* Matches Filter trackbar */
const char* matches_filter_name = "Matches Filter";
int matches_filter_max_value = 1;
int matches_filter_id = 0;

void matchesFiltering(int id)
{
	switch (id) {
	case 0:
	default: {
		matches_filter_name = "KNN filtering";
		curr_descriptor_matcher->knnMatch(descriptors_sample, descriptors_frame, knn_matches, 2);
		const float ratio_threshold = 0.7f;
		for (size_t i = 0; i < knn_matches.size(); i++)
		{
			if (knn_matches[i][0].distance < ratio_threshold * knn_matches[i][1].distance)
			{
				matches.push_back(knn_matches[i][0]);
			}
		}
	}
		   break;
	case 1: {
		matches_filter_name = "Score filtering";

		curr_descriptor_matcher->match(descriptors_sample, descriptors_frame, matches);

		size_t nb_matches = matches.size();
		double goodMatchRatio = 0.5;
		int nb_good_matches = (int)(nb_matches * goodMatchRatio);
		// Sort matches by score
		sort(matches.begin(), matches.end());
		// Filter out matches with large scores
		matches.erase(matches.begin() + nb_good_matches, matches.end());
	}
		  break;
	}
}

/* Opacity of Augmented image */
const char* opacity_name = "Opacity";
int opacity_max_value = 255;
int opacity_value = 128;

// signature of functions
void parseInput(int argc, char* argv[]); // convert input images
void init();
void createGUI(); // create trackbars to modify parameters
void update();
void callback(int value, void* userdata); // callback when parameters are modified
Mat overlay_image_bgr(Mat in_frame, Mat in_target);
void showResult(); // show generated images
void clearVectors(); // release space of pointers and list created
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
		clearVectors();

		if (waitKey(30) >= 0)
			break;
	}

	destroyAllWindows();

	return 0;
}

void parseInput(int argc, char* argv[])
{
	// this simple parser could only load two images -- target image & sample image
	if (argc >= 2 * 2 + 1)
		for (int i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "-i")) {
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
	}

	cvtColor(img_in[0], img_sample, COLOR_BGR2GRAY);
	img_target = img_in[1];
}

void init()
{
	/* webcam input */
	cap.open(0);
	cap >> frame;

	/* resize the target image to be the same as frame's size */
	resize(img_target, img_target, frame.size(), 0, 0, 0);

	/* Keypoint Detector pointers creation */
	akaze = AKAZE::create();
	orb = ORB::create();
	sift = SIFT::create();
	brisk = BRISK::create();
	surf = xfeatures2d::SURF::create();

	/* Descriptor Matcher pointers creation */
	BFL1_matcher = BFMatcher::create(NORM_L1);
	BFL2_matcher = BFMatcher::create(NORM_L2);
	BFHamming_matcher = BFMatcher::create(NORM_HAMMING);
	FLANN_based_matcher = FlannBasedMatcher::create();
}

void createGUI()
{
	/* put all parameters into settings window */
	namedWindow(WIN_SETTINGS_NAME, WINDOW_AUTOSIZE);

	/* if not in test mode, show only the optimal solution's parameters */
	if (TEST_MODE)
	{
		/* Keypoint Detector */
		createTrackbar(KPDetector_name, WIN_SETTINGS_NAME, &KPDetector_id,
			KPDetector_max_value, (TrackbarCallback)callback);
		/* Descriptor Matcher */
		createTrackbar(descriptor_matcher_name, WIN_SETTINGS_NAME, &descriptor_matcher_id,
			descriptor_matcher_max_value, (TrackbarCallback)callback);
		/* Matches Filter */
		createTrackbar(matches_filter_name, WIN_SETTINGS_NAME, &matches_filter_id,
			matches_filter_max_value, (TrackbarCallback)callback);
	}
	/* Opacity */
	createTrackbar(opacity_name, WIN_SETTINGS_NAME, &opacity_value,
		opacity_max_value, (TrackbarCallback)callback);
}

void update()
{
	/* update webcam input */
	cap >> frame;
	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

	/* KPDetector extraction */
	curr_KPDetector->detectAndCompute(img_sample, noArray(), keypoints_sample, descriptors_sample);
	curr_KPDetector->detectAndCompute(frame_gray, noArray(), keypoints_frame, descriptors_frame);

	drawKeypoints(img_sample, keypoints_sample, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	/* descriptor conversion */
	if (descriptor_matcher_id == 0) // for FLANN matcher
	{
		if (KPDetector_id == 2 || KPDetector_id == 0) // for ORB descriptors
		{
			descriptors_sample.convertTo(descriptors_sample, CV_32F);
			descriptors_frame.convertTo(descriptors_frame, CV_32F);
		}
	}

	/* descriptor matching */
	matchesFiltering(matches_filter_id);

	drawMatches(img_sample, keypoints_sample, frame_gray, keypoints_frame, matches, img_matches, Scalar::all(-1),
		Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	/* push into list of points that are in matches of sample image and frame */
	for (size_t i = 0; i < matches.size(); i++)
	{
		points_sample.push_back(keypoints_sample[matches[i].queryIdx].pt);
		points_frame.push_back(keypoints_frame[matches[i].trainIdx].pt);
	}

	/* four corners of sample image as point */
	corners_sample[0] = Point2f(0, 0);
	corners_sample[1] = Point2f((float)img_sample.cols, 0);
	corners_sample[2] = Point2f((float)img_sample.cols, (float)img_sample.rows);
	corners_sample[3] = Point2f(0, (float)img_sample.rows);

	/* perspective transformation of matching points */
	if (points_sample.size() != 0 && points_frame.size() != 0)
	{
		// define the form
		Mat H = findHomography(points_sample, points_frame, RANSAC);

		if (!H.empty())
		{
			warpPerspective(img_target, img_homography, H, img_homography.size(), 0);
			double alpha = (double)opacity_value / (double)opacity_max_value;
			double beta = 1 - alpha;

			img_overlay = overlay_image_bgr(frame, img_homography);

			addWeighted(img_overlay, alpha, frame, beta, 0.0, frame_augmented);
		}
		else
		{
			frame_augmented = frame;
		}
	}

	/* show result */
	showResult();
}

void callback(int value, void* userdata)
{
	cout << "------------------------------------------------------------------------------" << endl;
	curr_KPDetector = setKPDetector(KPDetector_id);
	cout << "> Keypoints Detector | " << KPDetector_name << endl;

	curr_descriptor_matcher = setDescriptorMatcher(descriptor_matcher_id);
	cout << "> Descriptor Matcher | " << descriptor_matcher_name << endl;

	matchesFiltering(matches_filter_id);
	cout << "> Matches Filtering  | " << matches_filter_name << endl;
}

Mat overlay_image_bgr(Mat in_frame, Mat in_target)
{
	Mat out = in_frame;
	int    i, j;
	int    nrows = in_frame.rows;
	int    ncols = in_frame.cols;
	int    nchannels_in = in_frame.channels();

	for (j = 0; j < nrows; j++) {
		for (i = 0; i < ncols; i++) {
			/*
			cout << (int)in_target.at<Vec3b>(0, 0)[0] << endl;
			cout << (int)in_target.at<Vec3b>(0, 0)[1] << endl;
			cout << (int)in_target.at<Vec3b>(0, 0)[2] << endl;
			*/
			if (in_target.at<Vec3b>(j, i)[0] > 0)
			{
				out.at<Vec3b>(j, i)[0] = in_target.at<Vec3b>(j, i)[0];
				out.at<Vec3b>(j, i)[1] = in_target.at<Vec3b>(j, i)[1];
				out.at<Vec3b>(j, i)[2] = in_target.at<Vec3b>(j, i)[2];
			}
		}
	}
	return out;
}

void showResult()
{
	//imshow(WIN_KEYPOINTS_NAME, img_keypoints);
	imshow(WIN_SETTINGS_NAME, img_matches);
	imshow(WIN_AUGMENTATION_NAME, frame_augmented);
}

void clearVectors()
{
	keypoints_sample.clear();
	keypoints_frame.clear();

	matches.clear();
	for (int i = 0; i < knn_matches.size(); i++)
	{
		knn_matches[i].clear();
	}
	knn_matches.clear();

	points_frame.clear();
	points_sample.clear();
	corners_frame.clear();
	corners_sample.clear();
}

//-----------------------------------------------------------------------------

int usage(char* prgname)
{
	cout << "This program should load " << 2 << " images as input" << endl;
	cout << "Usage: " << prgname << " ";
	cout << "[-i {image file}]" << endl;
	exit(-1);
}

void KPTests()
{
	clock_t t;
	t = clock();
	printf("Calculating...\n");

	surf->detect(img_sample, keypoints_sample);

	t = clock() - t;
	printf("surf took me %d clocks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	printf("keypoints numbers: %d.\n", (int)keypoints_sample.size());

	sift->detect(img_sample, keypoints_sample);

	t = clock() - t;
	printf("sift took me %d clocks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	printf("keypoints numbers: %d.\n", (int)keypoints_sample.size());

	orb->detect(img_sample, keypoints_sample);

	t = clock() - t;
	printf("orb took me %d clocks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	printf("keypoints numbers: %d.\n", (int)keypoints_sample.size());

	akaze->detect(img_sample, keypoints_sample);

	t = clock() - t;
	printf("akaze took me %d clocks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	printf("keypoints numbers: %d.\n", (int)keypoints_sample.size());

	brisk->detect(img_sample, keypoints_sample);

	t = clock() - t;
	printf("brisk took me %d clocks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	printf("keypoints numbers: %d.\n", (int)keypoints_sample.size());

	drawKeypoints(img_sample, keypoints_sample, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	imshow(WIN_KEYPOINTS_NAME, img_keypoints);
}

void calculateAverage()
{
	float total = 0;
	for (int i = 0; i < percentage.size(); i++)
	{
		total += percentage[i];
	}
	average = total / percentage.size();
	printf("average rate: %f\n", average);
}

void matchTest()
{
	clock_t t;
	t = clock();

	/* descriptor matching */
	matchesFiltering(matches_filter_id);

	t = clock() - t;
	printf("%s descriptors matching took me %d clocks (%f seconds).\n", KPDetector_name, t, ((float)t) / CLOCKS_PER_SEC);
	printf("The matcher is %s.\n", descriptor_matcher_name);
	printf("The matcher filter is %s.\n", matches_filter_name);
	printf("keypoints1 numbers: %d.\n", (int)keypoints_sample.size());
	printf("keypoints2 numbers: %d.\n", (int)keypoints_frame.size());
	printf("matching numbers: %d.\n", (int)matches.size());
	printf("matching rate: %f.\n", (float)matches.size() / keypoints_frame.size());
	percentage.push_back((float)matches.size() / (float)keypoints_frame.size());
}

//-----------------------------------------------------------------------------