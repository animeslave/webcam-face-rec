#ifndef FACESHAPES_HPP
#define FACESHAPES_HPP

// OpenCV
#include <opencv2/core/mat.hpp>

// Inner classes
#include "kalmanfilter.hpp"

namespace app
{

struct shape
{
	cv::Point predictedPosition;
	cv::Point actualPosition;
	cv::Point correctedPosition;
};

class FaceShapes
{
public:
	// Functions
	static void drawFaceShape();
	
	
	// Variables
	std::vector<shape> cheekToCheekShape;
	std::vector<shape> rightEyebrowShape;
	std::vector<shape> leftEyebrowShape;
	std::vector<shape> noseShape;
	std::vector<shape> rightEyeShape;
	std::vector<shape> leftEyeShape;
	std::vector<shape> outerMouthShape;
	std::vector<shape> innerMouthShape;
	
	
private:
	FaceShapes();
	~FaceShapes();
	
	// Functions
	
	
	
	// Variables
	
	
	
};

}

#endif // FACESHAPES_HPP
