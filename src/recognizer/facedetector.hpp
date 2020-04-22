#ifndef FACEDETECTOR_HPP
#define FACEDETECTOR_HPP

#include <vector>
#include <array>

// OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

// Inner classes
#include "landmarksdetector.hpp"

class FaceDetector
{
public:
	FaceDetector();
	~FaceDetector();
	
	template <typename T>
	std::vector<Face> detect_faces(const cv::Mat &frame);
	
private:
	// Variables
	cv::CascadeClassifier face_classifier;
	cv::CascadeClassifier eye_classifier;
	cv::CascadeClassifier lefteye_classifier;
	cv::CascadeClassifier righteye_classifier;
	LandmarksDetector* landmarksDetector;
	
	
	// Functions
	cv::Point get_nose_base(const Face &face);
	cv::Point get_nose_tip(const Face &face);
	cv::Point get_mouth_base(const Face &face);
	double get_theta(const Face &face);
	double get_tau(const Face &face);
	std::array<float,3> detect_face_gaze(const Face &face);
};

#endif // FACEDETECTOR_HPP