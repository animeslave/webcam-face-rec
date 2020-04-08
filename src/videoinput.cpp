#include "videoinput.hpp"

namespace app
{

VideoInput* VideoInput::instance = nullptr;
cv::VideoCapture* VideoInput::webcam = nullptr;

VideoInput::VideoInput(){}
VideoInput::~VideoInput(){}

VideoInput* VideoInput::getInstance() 
{
	if (instance == nullptr)
		instance = new VideoInput();
	return instance;
}

void VideoInput::releaseInstance()
{
	if (instance != nullptr)
		delete instance;
	instance = nullptr;
}

bool VideoInput::openCamera()
{
	// Initialize OpenCV webcam capture
	webcam = new cv::VideoCapture(0);
	if (!webcam->isOpened())
	{
		fprintf( stderr, "Unable to connect to camera.\n" );
		getchar();
		return false;
	}
	return true;
}

void VideoInput::grabFrame(cv::Mat* frame)
{
	*webcam >> *frame;
}

bool VideoInput::releaseCamera()
{
	// Release OpenCV webcam capture
	webcam->release();
	return true;
}

}

