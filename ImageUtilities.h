//
// File:		OpenCV-ImageHistogram-Utilities.h
// Author:		Nicolas ROUGON
// Affiliation:	Institut Mines-Telecom | Telecom SudParis | ARTEMIS Department
// Date:		June 9, 2019
//
// Description:	OpenCV sample routine
// > Miscellaneous image utilities
//

#include "opencv2/core.hpp"                // OpenCV core routines
#include "opencv2/highgui.hpp"             // OpenCV GUI routines
#include "opencv2/imgproc.hpp"             // OpenCV Image Processing routines
#include <iostream>

using namespace std;
using namespace cv;                        // OpenCV classes and routines

void FindZeroCrossings(Mat, Mat*);
void overlay_uchar_image(Mat, Mat, uchar, Vec3b, Mat*);
void vpad_and_hconcat(Mat, Mat, Scalar, Mat*);
