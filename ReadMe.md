# questions
1. descriptors "BruteForce-Hamming" not working with other algorithm, why ?
2. should I provide only the option of knn, or only normal match method, or both ?
3. how to do image augmentation?(warp & perspective transform)

# how to run the project
This project use opencv 4.4.0 version with opencv_contrib 4.4.0 version.

# motivation
For keypoints detector and descriptor, I choose SURF(Speeded-Up Robust Features) way.

## Harris Corner Detector and Shi-Tomasi Corner Detector
**Harris Corner Detector** and **Shi-Tomasi Corner Detector** are both scale variant detectors, which are not suitable in case that the target has been scaled.

## SIFT
**Scale Invariant Feature Transform** is scale-invariant, good at handling illumination change as well.
But it costs more time than SURF(about 3 times).


## SURF
 **SURF** approximates LoG with Box Filter convolution, which is faster and is suitable for real-time detecting task.
 SURF is good at handling images with blurring and rotation, but not good at handling viewpoint change and illumination change.
 

## FAST
It is several times faster than other existing corner detectors.
But it is not robust to high levels of noise. It is dependent on a threshold.
The **detectAndCompute** is not implemented for this feature detector.

## BRIEF
BRIEF is only a feature descriptor, not a detector.
SIFT uses 128-dim vector for descriptors. SURF also takes minimum of 256 bytes (for 64-dim). 
These floating-point data can be converted as binary string and match features using Hamming distance(faster).

## ORB Oriented FAST and Rotated BRIEF
ORB is basically a fusion of FAST keypoint detector and BRIEF descriptor with many modifications to enhance the performance,
with rotation invariant.
The paper says ORB is much faster than SURF and SIFT and ORB descriptor works better than SURF.
It has a number of optional parameters. Most useful ones are nFeatures which denotes maximum number of features to be retained (by default 500),
scoreType which denotes whether Harris score or FAST score to rank the features (by default, Harris score) etc. Another parameter,
WTA_K decides number of points that produce each element of the oriented BRIEF descriptor. 
By default it is two, ie selects two points at a time. In that case, for matching, NORM_HAMMING distance is used.
If WTA_K is 3 or 4, which takes 3 or 4 points to produce BRIEF descriptor, then matching distance is defined by NORM_HAMMING2.
# references
opencv Feature Detection and Description
https://docs.opencv.org/3.4/db/d27/tutorial_py_table_of_contents_feature2d.html
