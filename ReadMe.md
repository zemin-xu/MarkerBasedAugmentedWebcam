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

# references
opencv Feature Detection and Description
https://docs.opencv.org/3.4/db/d27/tutorial_py_table_of_contents_feature2d.html
