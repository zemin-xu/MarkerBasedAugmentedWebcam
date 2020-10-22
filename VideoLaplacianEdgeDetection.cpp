////
//// File:	VideoLaplacianEdgeDetection.cpp
//// Author:	Nicolas ROUGON
//// Affiliation:	Institut Mines-Telecom / Telecom SudParis / ARTEMIS Department
//// Date:	July 10, 2020
//// Description:	OpenCV sample routine
//// > Laplacian-based edge detection from camera video stream
////
//// Documentation:
//// - The VideoCapture class
////   > docs.opencv.org/4.1.0/d8/dfe/classcv_1_1VideoCapture.html
//// - Gaussian filtering
////   > docs.opencv.org/4.1.0/d4/d86/group__imgproc__filter.html#gaabe8c836e97159a9193fb0b11ac52cf1
//// - Finite Difference Laplacian operator
////   > docs.opencv.org/4.1.0/d4/d86/group__imgproc__filter.html#gad78703e4c8fe703d479c1860d76429e6
//// Tutorial:
//// - How to read a video stream
////   > docs.opencv.org/4.1.0/d5/dc4/tutorial_video_input_psnr_ssim.html
//// - Laplacian operator
////   > docs.opencv.org/4.1.0/d5/db5/tutorial_laplace_operator.html
//
//#include "opencv2/core.hpp"                // OpenCV core routines
//#include <opencv2/videoio.hpp>             // OpenCV video routines
//#include "opencv2/highgui.hpp"             // OpenCV GUI routines
//#include "opencv2/imgproc.hpp"             // OpenCV Image Processing routines
//#include <iostream>
//
//using namespace std;
//using namespace cv;                        // OpenCV classes and routines
//
//// Constants
//enum { L1NORM, L2NORM };
//enum { STD_DEVIATION, VARIANCE };
//enum { LoG, DoG };
//
////---- Global variables
//Mat frame, frame_gray, frame_gray32f, frame_smoothed, frame_overlay;
//Mat* the_frame;
//Mat contrast_map, high_contrast_mask;
//Mat laplacian_map, laplacian_ZC;
//
//// Camera video stream
//VideoCapture cap;
//bool isGrayCamera = false;
//
//// Laplacian filter type
//const  char* trackbarLaplacianFilter_name = "LoG | DoG";
//int    trackbarLaplacianFilter_max_value = 1;
//int    trackbarLaplacianFilter_value;      // Trackbar current values
//int    LaplacianFilter_default_value = 0;  // Default kernel half size
//int    laplacian_filter_type = DoG;        // Laplacian filter type (LoG | DoG)
//
//// Gaussian smoothing
//const  char* trackbarStdDeviation_name = "10*StdDev";
//int    trackbarStdDeviation_max_value = 75;
//int    trackbarStdDeviation_value;         // Trackbar value
//double trackbarStdDeviation_stepsize = 0.1;  // Quantization step
//double sigma = 1.0;                        // Gaussian kernel std deviation
//
//// Laplacian kernel half-size
//const  char* trackbarKernelHalfSize_name = "Half-width";
//int    trackbarKernelHalfSize_max_value = 8;
//int    trackbarKernelHalfSize_value;       // Trackbar current values
//int    KernelHalfSize_default_value = 3;   // Default kernel half size
//int    laplacian_ksize;                    // Kernel size
//
//// DoG filter
//double DoGStdDeviationRatio = 1.6;         // Ratio of Gaussian std deviations
//int    gaussian_ksize1, gaussian_ksize2;   // Gaussian kernel sizes
//double sigma2;                             // Larger Gaussian kernel std deviation
//
//// Contrast estimator
//const char* trackbarContrastEstimator_name = "Grad | Var";  // Trackbar name
//int trackbarContrastEstimator_max_value = 1;  // Trackbar max value
//int trackbarContrastEstimator_value;       // Trackbar current value
//int contrast_estimator = 0;                // Estimator parameter
//
//// Contrast threshold
//const char* trackbarContrastThreshold_name = "Threshold";  // Trackbar name
//int trackbarContrastThreshold_max_value = 255;  // Trackbar max value
//int trackbarContrastThreshold_value;       // Trackbar current value
//double contrast_threshold_value = 128.;    // Threshold parameter
//
//// Edge map overlay
//uchar edge_color = 255;                    // Edge intensity (Gray overlay)
//Vec3b edge_color3 = Vec3b(0, 0, 255);        // Edge color (BGR overlay)
//
//// Windows
//char window_out_name[256];                 // Display edge map onto original image
//
////---- Routines
//#include "ImageUtilities.h"
//
//void create_GUI();
//void EdgeDetection_callback(int, void*);
//void EdgeDetection();
//
//void DoGFilter(Mat, double, double, int, Mat*);
//void LocalVarianceMap(Mat, int, int, Mat*);
//void SobelContrastMap(Mat, int, Mat*);
//
////-----------------------------------------------------------------------------
//
//int main(int argc, char* argv[])
//{
//	// Open default camera
//	cap.open(0);
//
//	// Check camera opening is successful
//	if (!cap.isOpened())
//		return -1;
//
//	// Get camera properties and initialize various variables
//	// - Get frame dimensions and create edge overlay image
//	Size frameSize = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
//		(int)cap.get(CAP_PROP_FRAME_HEIGHT));
//
//	laplacian_map.create(frameSize, CV_32F);
//	laplacian_ZC.create(frameSize, CV_8U);
//	contrast_map.create(frameSize, CV_32F);
//	high_contrast_mask.create(frameSize, CV_8U);
//	frame_overlay.create(frameSize, CV_8UC3);
//
//	// - Check for color camera
//	cap >> frame;                                 // Get a new frame
//	if (frame.channels() == 1)
//		isGrayCamera = true;
//
//	// Create GUI
//	create_GUI();
//
//	// Invoke callback routine to initialize
//	EdgeDetection_callback(trackbarStdDeviation_value, 0);
//
//	// Perform edge detection from video stream
//	for (;;) {
//		EdgeDetection();
//
//		// Listen to next event - Exit if key pressed
//		if (waitKey(30) >= 0)
//			break;
//	}
//
//	// Destroy windows
//	destroyAllWindows();
//
//	// The camera will be released automatically in VideoCapture destructor
//	return 0;
//}
//
////-----------------------------------------------------------------------------
//
//void create_GUI()
//{
//	const char* window_name_prefix = "OpenCV Demo | Edge detection >";
//
//	// Create window for edge map overlay
//	sprintf(window_out_name, "%s Detected edges", window_name_prefix);
//	namedWindow(window_out_name, WINDOW_AUTOSIZE);
//
//	// Create trackbars
//	// - for Laplacian filter type (LoG | DoG)
//	trackbarLaplacianFilter_value = LaplacianFilter_default_value;
//	createTrackbar(trackbarLaplacianFilter_name, window_out_name,
//		&trackbarLaplacianFilter_value,
//		trackbarLaplacianFilter_max_value,
//		(TrackbarCallback)EdgeDetection_callback);
//
//	// - for Gaussian kernel std deviation
//	trackbarStdDeviation_value = (int)(sigma / trackbarStdDeviation_stepsize);
//	createTrackbar(trackbarStdDeviation_name, window_out_name,
//		&trackbarStdDeviation_value,
//		trackbarStdDeviation_max_value,
//		(TrackbarCallback)EdgeDetection_callback);
//
//	// - for Laplacian kernel half-width (LoG filter)
//	trackbarKernelHalfSize_value = KernelHalfSize_default_value;
//	createTrackbar(trackbarKernelHalfSize_name, window_out_name,
//		&trackbarKernelHalfSize_value,
//		trackbarKernelHalfSize_max_value,
//		(TrackbarCallback)EdgeDetection_callback);
//
//	// - for contrast estimator
//	trackbarContrastEstimator_value = contrast_estimator;
//	createTrackbar(trackbarContrastEstimator_name, window_out_name,
//		&trackbarContrastEstimator_value,
//		trackbarContrastEstimator_max_value,
//		(TrackbarCallback)EdgeDetection_callback);
//
//	// - for contrast threshold
//	trackbarContrastThreshold_value = (int)contrast_threshold_value;
//	createTrackbar(trackbarContrastThreshold_name, window_out_name,
//		&trackbarContrastThreshold_value,
//		trackbarContrastThreshold_max_value,
//		(TrackbarCallback)EdgeDetection_callback);
//}
//
////-----------------------------------------------------------------------------
//
//void EdgeDetection_callback(int value, void* userdata)
//{
//	// Set hyperparameter values from trackbars
//	if (trackbarLaplacianFilter_value == 0)
//		laplacian_filter_type = LoG;
//	else if (trackbarLaplacianFilter_value == 1)
//		laplacian_filter_type = DoG;
//	sigma = ((double)trackbarStdDeviation_value) * trackbarStdDeviation_stepsize;
//	sigma2 = sigma * DoGStdDeviationRatio;
//	laplacian_ksize = 2 * trackbarKernelHalfSize_value + 1;
//
//	contrast_estimator = trackbarContrastEstimator_value;
//	contrast_threshold_value = (double)trackbarContrastThreshold_value;
//
//	// Gradient-based and variance-based contrasts have different dynamics.
//	// This issue is fixed by rescaling the threshold parameter
//	if (contrast_estimator == 1)    // Local std deviation
//		contrast_threshold_value *= 0.1;
//
//	// Ckeck/fix trackbars to forbid inconsistent values.
//	// Setting trackbar position triggers the callback again
//	if (trackbarStdDeviation_value == 0) {
//		setTrackbarPos(trackbarStdDeviation_name, window_out_name, 1);
//		return;
//	}
//
//	if (trackbarKernelHalfSize_value == 0) {
//		setTrackbarPos(trackbarKernelHalfSize_name, window_out_name, 1);
//		return;
//	}
//
//	// Update edge map
//	EdgeDetection();
//}
//
////-----------------------------------------------------------------------------
//// Gradient-based edge detection
//// - The processing pipeline is the same as in OpenCV-LaplacianEdgeDetection.cpp
//
//void EdgeDetection()
//{
//	// Get a new frame from the camera
//	cap >> frame;
//
//	// Convert frame to graylevel if appropriate
//	if (isGrayCamera)
//		the_frame = &frame;
//	else {
//		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
//		the_frame = &frame_gray;
//	}
//
//	// Preprocessing > Gaussian filtering
//	// - Kernel size is set to 0, and is automatically estimated from sigma
//	// - Output image has the same type as the input image.
//	//   Computations can be performed in CV_8U with minimum truncation error.
//	//   For the sake of accuracy, it is better to convert input frame to CV_32F
//	//
//	the_frame->convertTo(frame_gray32f, CV_32F);
//	GaussianBlur(frame_gray32f, frame_smoothed, Size(0, 0), sigma, sigma);
//
//	// Compute regularized Laplacian map
//	if (laplacian_filter_type == LoG)
//		Laplacian(frame_smoothed, laplacian_map, CV_32F, laplacian_ksize);
//	else if (laplacian_filter_type == DoG)
//		DoGFilter(frame_gray32f, sigma, sigma2, 1, &laplacian_map);
//
//	// Detect Laplacian zero-crossings
//	FindZeroCrossings(laplacian_map, &laplacian_ZC);
//
//	// Compute contrast map
//	if (contrast_estimator == 0)          // Sobel gradient norm
//		SobelContrastMap(frame_smoothed, L2NORM, &contrast_map);
//	else if (contrast_estimator == 1)    // Local std deviation
//		LocalVarianceMap(frame_smoothed, 3, STD_DEVIATION, &contrast_map);
//
//	// Threshold contrast map
//	threshold(contrast_map, high_contrast_mask,
//		contrast_threshold_value, 255, THRESH_BINARY);
//
//	// Filter Laplacian zero-crossings based on contrast
//	high_contrast_mask.convertTo(high_contrast_mask, CV_8U);
//	bitwise_and(high_contrast_mask, laplacian_ZC, laplacian_ZC);
//
//	// Overlay edge map on original image
//	overlay_uchar_image(frame, laplacian_ZC,
//		edge_color, edge_color3, &frame_overlay);
//
//	// Display resulting image
//	imshow(window_out_name, frame_overlay);
//}
//
////-----------------------------------------------------------------------------
//// Compute contrast map using Sobel gradient filter
//
//void SobelContrastMap(Mat in, int norm, Mat* out)
//{
//	Mat gradx, grady;
//
//	// Compute Sobel gradient components
//	Sobel(in, gradx, CV_32F, 1, 0);
//	Sobel(in, grady, CV_32F, 0, 1);
//
//	// Compute contrast map
//	if (norm == L1NORM)
//		*out = abs(gradx) + abs(grady);
//	else if (norm == L2NORM)
//		sqrt(gradx.mul(gradx) + grady.mul(grady), *out);
//
//	// (Safety) Memory deallocation
//	gradx.release();
//	grady.release();
//}
//
////-----------------------------------------------------------------------------
//// Compute local variance map:   sigma^2(I) = E(I^2) - (E(I))^2
//// - Expectation is implemented using a normalized avering filter blur()
//// Documentation: docs.opencv.org/2.4/modules/imgproc/doc/filtering.html#blur
//
//void LocalVarianceMap(Mat in, int ksize, int type, Mat* out)
//{
//	Mat  in32f, in_mean, in2_mean;
//	Mat* p_in;
//
//	// Convert image to float array
//	if (in.type() != CV_32F) {
//		in.convertTo(in32f, CV_32F);
//		p_in = &in32f;
//	}
//	else
//		p_in = &in;
//
//	// Compute image local mean
//	blur(*p_in, in_mean, Size(ksize, ksize));
//
//	// Compute image squared local mean
//	blur(p_in->mul(*p_in), in2_mean, Size(ksize, ksize));
//
//	// Compute local variance map
//	switch (type)
//	{
//	case VARIANCE:
//		*out = in2_mean - in_mean.mul(in_mean);
//		break;
//	case STD_DEVIATION:
//	default:
//		sqrt(in2_mean - in_mean.mul(in_mean), *out);
//		break;
//	}
//
//	// Memory deallocation (safe)
//	if (in.type() != CV_32F)
//		in32f.release();
//	in_mean.release();
//	in2_mean.release();
//}
//
////-----------------------------------------------------------------------------
//// Difference of Gaussians filter
//// - The DoG filter is linear but NOT separable
//
//void DoGFilter(Mat in, double sigma1, double sigma2, int method, Mat* out)
//{
//	Mat  in32f, out1, out2;
//	Mat* p_in;
//
//	// The image returned by GaussianBlur() has the same type as the input image
//	// Therefore, first convert input image into CV_32F if appropriate
//	if (in.type() != CV_32F) {
//		in.convertTo(in32f, CV_32F);
//		p_in = &in32f;
//	}
//	else
//		p_in = &in;
//
//	int depth = in32f.depth();
//
//	// Gaussian filtering
//	switch (method)
//	{
//	case 0:    // Kernel size not specified
//	  // GaussianBlur() computes kernel size as:
//	  //   ksize = cvRound(sigma*(in.depth == CV_8U ? 3 : 4)*2 + 1) | 1;
//	  // Source code: $(OPENCVDIR)/sources/modules/imgproc/src/smooth.cpp
//
//		gaussian_ksize1 = cvRound(sigma1 * (depth == CV_8U ? 3 : 4) * 2 + 1) | 1;
//		gaussian_ksize2 = cvRound(sigma2 * (depth == CV_8U ? 3 : 4) * 2 + 1) | 1;
//
//		GaussianBlur(*p_in, out1, Size(0, 0), sigma1, sigma1);
//		GaussianBlur(*p_in, out2, Size(0, 0), sigma2, sigma2);
//		break;
//	case 1:    // Optimal kernel size is computed, yielding narrower kernels
//	  // Optimal Gaussian kernel size is given by:
//	  //            sigma = 0.3*((ksize-1)*0.5 - 1) + 0.8
//	  // i.e.       ksize = 2*((sigma-0.8)/0.3) + 3
//	  // Documentation : docs.opencv.org/2.4/modules/imgproc/doc/filtering.html#getgaussiankernel
//
//		gaussian_ksize1 = ((int)ceil((sigma1 - 0.8) / 0.3)) * 2 + 3;
//		gaussian_ksize2 = ((int)ceil((sigma2 - 0.8) / 0.3)) * 2 + 3;
//		GaussianBlur(*p_in, out1, Size(gaussian_ksize1, gaussian_ksize1), sigma1, sigma1);
//		GaussianBlur(*p_in, out2, Size(gaussian_ksize2, gaussian_ksize2), sigma2, sigma2);
//		break;
//	}
//
//	// Subtract filter outputs
//	*out = out1 - out2;
//
//	// Memory deallocation (safe)
//	if (in.type() != CV_32F)
//		in32f.release();
//	out1.release();
//	out2.release();
//}
//
////-----------------------------------------------------------------------------