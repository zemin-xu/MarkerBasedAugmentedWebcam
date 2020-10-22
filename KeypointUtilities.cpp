//
// File:		OpenCV-Keypoint-Utilities.cpp
// Author:		Nicolas ROUGON
// Affiliation:	Institut Mines-Telecom | Telecom SudParis | ARTEMIS Department
// Date:		June 30, 2019
//
// Description:	OpenCV sample routine
// > Miscellaneous keypoint detection & matching utilities
//

#include "opencv2/core.hpp"                // OpenCV core routines
#include "opencv2/highgui.hpp"             // OpenCV GUI routines
#include "opencv2/imgproc.hpp"             // OpenCV Image Processing routines
#include "opencv2/calib3D.hpp"             // OpenCV Multiview geometry routines
#include <iostream>

using namespace std;
using namespace cv;                        // OpenCV classes and routines

// Routines
#include "KeypointUtilities.h"

//-----------------------------------------------------------------------------

void set_KP_location(vector<KeyPoint>keypoints[],
	vector<DMatch>matches, int nb_matches,
	vector<Point2f>matched_keypoints[])
{
	for (size_t i = 0; i < nb_matches; i++) {
		matched_keypoints[0].push_back(keypoints[0][matches[i].queryIdx].pt);
		matched_keypoints[1].push_back(keypoints[1][matches[i].trainIdx].pt);
	}
}

//-----------------------------------------------------------------------------

void set_image_support(Mat image, vector<Point2f>* support)
{
	support->push_back(Point2f(0, 0));
	support->push_back(Point2f((float)image.cols, 0));
	support->push_back(Point2f((float)image.cols, (float)image.rows));
	support->push_back(Point2f(0, (float)image.rows));
}

//-----------------------------------------------------------------------------

void draw_quadrilateral(vector<Point2f> vertices, int shiftx,
	Scalar color, int line_thickness, Mat image)
{
	// Apply horizontal shift
	for (size_t i = 0; i < 4; i++)
		vertices[i].x += shiftx;

	// Draw box
	line(image, vertices[0], vertices[1], color, line_thickness);
	line(image, vertices[1], vertices[2], color, line_thickness);
	line(image, vertices[2], vertices[3], color, line_thickness);
	line(image, vertices[3], vertices[0], color, line_thickness);
}

//-----------------------------------------------------------------------------

void set_DescriptorMatcher_name(int id, char* name, char* metric_name)
{
	switch (id) {
	case 0:  // ORB
	case 1:  // AKAZE;
	case 2:  // BRISK
	default:
		sprintf(name, "Brute Force");
		sprintf(metric_name, "Hamming");
		break;
	case 3: // KAZE
		sprintf(name, "Brute Force");
		sprintf(metric_name, "L2");
		break;
	}
}

//-----------------------------------------------------------------------------

void set_KPdetector_name(int id, char* name)
{
	switch (id) {
	case 0:
	default:
		sprintf(name, "ORB");
		break;
	case 1:
		sprintf(name, "AKAZE");
		break;
	case 2:
		sprintf(name, "BRISK");
		break;
	case 3:
		sprintf(name, "KAZE");
		break;
	}
}

//-----------------------------------------------------------------------------

void set_match_filter_scheme_name(int id, char* name)
{
	switch (id) {
	case 0:
	default:
		sprintf(name, "%% nearest");
		break;
	case 1:
		sprintf(name, "1-NN to 2-NN distance ratio");
		break;
	case 2:
		sprintf(name, "Max distance");
		break;
	}
}

//-----------------------------------------------------------------------------

void set_transform_estimator_name_type(int id, char* name, int* type)
{
	switch (id) {
	case 0:
		*type = 0;
		sprintf(name, "Least-Square");
		break;
	case 1:
		*type = cv::LMEDS;
		sprintf(name, "Least-Median");
		break;
	case 2:
	default:
		*type = cv::RANSAC;
		sprintf(name, "RANSAC");
		break;
	case 3:
		*type = cv::RHO;
		sprintf(name, "PROSAC");
		break;
	}
}

//-----------------------------------------------------------------------------

void set_transform_type_name(int id, char* name)
{
	switch (id) {
	case 0:
		sprintf(name, "None");
		break;
	case 1:
		sprintf(name, "2D Affinity");
		break;
	case 2:
	default:
		sprintf(name, "3D Homography");
		break;
	}
}

//-----------------------------------------------------------------------------

void display_detection_results(vector<KeyPoint> keypoints[], int nviews,
	char* window_name,
	const char* window_title_prefix,
	bool verbosity)
{
	int  k;
	char window_title[256];

	// Display keypoint detection results
	// - in command window
	if (verbosity == true) {
		cout << "> Keypoints";
		for (k = 0; k < nviews; k++)
			cout << " | View #" << k + 1 << ": " << keypoints[k].size();
		cout << endl;
	}

	// - in window titlebar
	sprintf(window_title, "%s Detection > Keypoints", window_title_prefix);
	for (k = 0; k < nviews; k++)
		sprintf(window_title, "%s | View #%d: %d",
			window_title, k + 1, (int)keypoints[k].size());
	setWindowTitle(window_name, window_title);
}

//-----------------------------------------------------------------------------

void display_matching_results(vector<DMatch> matches,
	int nb_matches, int nb_good_matches,
	int match_filter_id,
	char* window_name,
	const char* window_title_prefix,
	bool verbosity)
{
	char window_title[256];

	// Display keypoint matching results
	// - in command window
	if (verbosity == true) {
		cout << "> Matches | Found: " << nb_matches;
		cout << " | Good: " << nb_good_matches;
		cout << " (ratio: " << ((float)nb_good_matches) / ((float)nb_matches);
		cout << " | distance max: " << matches[nb_good_matches - 1].distance << ")" << endl;
		switch (match_filter_id) {
		case 0:
			cout << "  Distance bounds: [" << matches[0].distance << ", " << matches[nb_good_matches - 1].distance << "]" << endl;
			break;
		case 1:
		default:
			break;
		}
	}

	// - in window titlebar
	sprintf(window_title, "%s Matching > Matches", window_title_prefix);
	sprintf(window_title, "%s | Found: %d", window_title, nb_matches);
	sprintf(window_title, "%s | Good: %d", window_title, nb_good_matches);
	sprintf(window_title, "%s (ratio: %.2f", window_title,
		((float)nb_good_matches) / ((float)nb_matches));
	sprintf(window_title, "%s | distance max: %.1lf", window_title,
		matches[nb_good_matches - 1].distance);
	switch (match_filter_id) {
	case 0:
		sprintf(window_title, "%s bounds: [%.1lf, %.1lf]", window_title,
			matches[0].distance, matches[nb_good_matches - 1].distance);
		break;
	case 1:
	default:
		break;
	}
	sprintf(window_title, "%s)", window_title);
	setWindowTitle(window_name, window_title);
}

//-----------------------------------------------------------------------------