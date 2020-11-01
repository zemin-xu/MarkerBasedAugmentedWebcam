# Marker-based Augmented Reality model report

## Problem statement

In this project, I need to design and implement an OpenCV solution. The solution relies on keypoint-based image descriptor to match between the image of a structured planar marker and its instance in webcam stream. In order to have a optimal solution under webcam stream utilization, there are several factors to consider about.

1. **Computation Time**
This factor is the most curcial one because in the solution, the detection and matching task should be finished less than 1 frame.
The minimum value should be 30 frames per second so that the streaming is fluent enough, that's is to say, the maximum computation time should be less than 0.03 second for each frame.
2. **Rotation & Scale Invariance**
It is normal that the instance in webcam will be rotated or shown in different size. Therefore, a feature detector that has rotational variance or scale variance should not be considered.
3. **Light Condition & Motion Blur**
Sometimes the situation that the lighting is not ideal will happen or the instance is moving in front of camera. These factors should be also considered.
4. **Distinctiveness**
5. **Robustness**

In this project, I choose a hard book cover with rich details and another with much less interest points as objects to test.
I choose **SIFT**, **SURF**, **ORB**, **AKAZE** and **BRISK** as the feature detectors and carry out the experiments.
The **Harris Corner Detector**[1] and **Shi-Tomasi Corner Detector**[2] is eliminated because of their scale-variant feature.

## feature descriptors theory

### SIFT(Scale Invariant Feature Transform)

SIFT[3] is scale-invariant, which is also good at handling illumination change as well. It uses DoG as an approximation of LoG to calculate local extrema(potential keypoint) over scale[4]. SIFT provides distinctiveness, robustness and invariance to common image transformations such as rotation and scale. However such a robustness comes at a considerable computational cost[5].

### SURF(Speeded-Up Robust Features)

SURF[6] approximates LoG with Box Filter convolution calculation, which is faster and is suitable for real-time detecting task. SURF is good at handling images with blurring and rotation, but not good at handling viewpoint change and illumination change.[7] Its implementation is in opencv_contrib repository.

### ORB(Oriented FAST and Rotated BRIEF)

ORB[8] is a binary feature descriptor that came from *OpenCV Lab* and is basically a fusion of FAST[9] keypoint detector and BRIEF[10] descriptor with many modifications to enhance the performance and scale as well as rotational invariance[11]. It employs the efficient Hamming distance metric for matching and these features make it suitable for real-time feature detection and description.
One of its parameters is *nFeatures* which denotes maximum number of features to be retained.

### AKAZE

AKAZE[12] is the accelerated version of KAZE[13] which uses non-linear scale space to find features. It uses a binary descriptor that exploits gradient infomation. It is proved to be an improvement in repeatability and distinctiviness compared to previous algorithms.

### BRISK

BRISK[14]

## Experiment

To have a optimal solution, I firstly evaluate the time for detection and matching operations of algorithms above. Each detector pointer is created with default parameters which are supposed to be the optimal ones.

### performance


## References

1. [OPENCV -- Harris Corner Detection](https://docs.opencv.org/3.4/dc/d0d/tutorial_py_features_harris.html)

2. [OPENCV -- Shi-Tomasi Corner Detector & Good Features to Track](https://docs.opencv.org/3.4/d4/d8c/tutorial_py_shi_tomasi.html)

3. G. D. Lowe. Distinctive image features from scale-invariant keypoints. IJCV, pages 91–110, November 2004

4. [OPENCV -- Introduction to SIFT (Scale-Invariant Feature Transform)](https://docs.opencv.org/3.4/da/df5/tutorial_py_sift_intro.html)

5. A. Pieropan, M. Björkman, N. Bergström and D. Kragic, Feature Descriptors for Tracking by Detection: a Benchmark, Jul 2016, [online](https://arxiv.org/pdf/1607.06178.pdf)

6. Bay, H., Ess, A., Tuytelaars, T., Gool, L.V.: Surf: Speeded Up Robust Features. Computer Vision and Image Understanding 10, 346–359 (2008)

7. [OPENCV -- Introduction to SURF (Speeded-Up Robust Features)](https://docs.opencv.org/3.4/df/dd2/tutorial_py_surf_intro.html)

8. Ethan Rublee, Vincent Rabaud, Kurt Konolige, Gary R. Bradski: ORB: An efficient alternative to SIFT or SURF. ICCV 2011: 2564-2571.

9. Edward Rosten and Tom Drummond, “Machine learning for high speed corner detection” in 9th European Conference on Computer Vision, vol. 1, 2006, pp. 430–443.

10. Michael Calonder, Vincent Lepetit, Christoph Strecha, and Pascal Fua, "BRIEF: Binary Robust Independent Elementary Features", 11th European Conference on Computer Vision (ECCV), Heraklion, Crete. LNCS Springer, September 2010.

11. [OPENCV -- ORB (Oriented FAST and Rotated BRIEF)](https://docs.opencv.org/3.4/d1/d89/tutorial_py_orb.html)

12. KAZE Features. Pablo F. Alcantarilla, Adrien Bartoli and Andrew J. Davison. In European Conference on Computer Vision (ECCV), Fiorenze, Italy, October 2012. bibtex

13. Fast Explicit Diffusion for Accelerated Features in Nonlinear Scale Spaces. Pablo F. Alcantarilla, Jesús Nuevo and Adrien Bartoli. In British Machine Vision Conference (BMVC), Bristol, UK, September 2013. bibtex

https://arxiv.org/ftp/arxiv/papers/1710/1710.02726.pdf

# how to run the project

This project use opencv 4.4.0 version with opencv_contrib 4.4.0 version.

