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

const int NUM_IMG_IN = 2;
const char* WIN_SETTINGS_NAME = "Settings";
const char* WIN_OUT_NAME = "Output";

char  input_img_paths[NUM_IMG_IN][256];

Mat img_in[NUM_IMG_IN];

Mat img_example, img_target;

Mat frame; // realtime webcam frame
Mat frame_gray, frame_gray32f, frame_smoothed, frame_overlay;

Mat img_matches;

Mat* image_composite;

/* pointers to feature extractor */
Ptr<AKAZE> akaze;
Ptr<ORB> orb;
Ptr<SIFT> sift;
Ptr<Feature2D> curr_KPDetector;

/* Keypoint detector trackbar */
const char* KPDetector_name = "KPDetector";
int KPDetector_max_value = 2;
int KPDetector_value;
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
int descriptor_matcher_value;
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

Mat descriptors1, descriptors2;

vector<KeyPoint> keypoints1, keypoints2;

vector<Point2f>support;
vector<Point2f>registered_support;

VideoCapture cap;
bool isGrayCamera = false;

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

	cout << "init" << endl;
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
	if (argc >= NUM_IMG_IN * 2 + 1)
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
	for (int i = 0; i < NUM_IMG_IN; i++) {
		// Load image
		img_in[i] = imread(input_img_paths[i], IMREAD_UNCHANGED);
		if (img_in[i].empty()) {
			cout << "!!! Cannot read the image " << input_img_paths[i] << " !!!" << endl;
		}
	}

	img_example = img_in[0];
	img_target = img_in[1];
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
	KPDetector_value = (int)KPDetector_id;
	createTrackbar(KPDetector_name, WIN_SETTINGS_NAME, &KPDetector_value,
		KPDetector_max_value, (TrackbarCallback)callback);
	/* Descriptor Matcher */
	descriptor_matcher_value = (int)descriptor_matcher_id;
	createTrackbar(descriptor_matcher_name, WIN_SETTINGS_NAME, &descriptor_matcher_value,
		descriptor_matcher_max_value, (TrackbarCallback)callback);
}

void update()
{
	// update webcam input
	cap >> frame;
	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

	/* feature detection */
	curr_KPDetector->detectAndCompute(img_example, noArray(), keypoints1, descriptors1);
	curr_KPDetector->detectAndCompute(frame, noArray(), keypoints2, descriptors2);

	/* feature description */
	std::vector< std::vector<DMatch> > knn_matches;
	FLANN_based_matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);
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
	warpPerspective(img_example, frame, H, img_example.size());
	//-- Get the corners from the image_1 ( the object to be "detected" )

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

	//Mat* the_image = &img_target;

	// Concatenate source and fused images horizontally
	//hconcat(img_example, *the_image, *image_composite);

	// Display result
	//imshow("out", *image_composite);

	//-- Show detected matches
	//imshow("Good Matches & Object detection", img_matches);

	// - Keypoint matching
	imshow(WIN_SETTINGS_NAME, img_matches);
}

void callback(int value, void* userdata)
{
	cout << "------------------------------------------------------------------------------" << endl;
	KPDetector_id = (int)KPDetector_value;
	curr_KPDetector = setKPDetector(KPDetector_id);
	cout << "> Keypoints Detector | " << KPDetector_name << endl;

	descriptor_matcher_id = (int)descriptor_matcher_value;
	curr_descriptor_matcher = setDescriptorMatcher(descriptor_matcher_id);
	cout << "> Descriptor Matcher | " << descriptor_matcher_name << endl;
}

//-----------------------------------------------------------------------------

int usage(char* prgname)
{
	cout << "This program should load " << NUM_IMG_IN << " images as input" << endl;
	cout << "Usage: " << prgname << " ";
	cout << "[-i {image file}]" << endl;
	exit(-1);
}

//-----------------------------------------------------------------------------