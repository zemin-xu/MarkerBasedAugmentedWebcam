//
// File:		OpenCV-Keypoint-Utilities.h
// Author:		Nicolas ROUGON
// Affiliation:	Institut Mines-Telecom | Telecom SudParis | ARTEMIS Department
// Date:		June 30, 2019
//
// Description:	OpenCV sample routine
// > Miscellaneous keypoint detection & matching utilities
//

void display_detection_results(vector<KeyPoint>[], int,
	char*, const char*, bool);
void display_matching_results(vector<DMatch>, int, int, int,
	char*, const char*, bool);
void draw_quadrilateral(vector<Point2f>, int, Scalar, int, Mat);
void set_DescriptorMatcher_name(int, char*, char*);
void set_image_support(Mat, vector<Point2f>*);
void set_KPdetector_name(int, char*);
void set_KP_location(vector<KeyPoint>[], vector<DMatch>,
	int, vector<Point2f>[]);
void set_match_filter_scheme_name(int, char*);
void set_transform_estimator_name_type(int, char*, int*);
void set_transform_type_name(int, char*);
