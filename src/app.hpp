#ifndef APP_H
#define APP_H

#include <thread>

// Live2D


// OpenCV
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// CMUSphinx
//#include <pocketsphinx/pocketsphinx.h>

// Inner classes
#include "recognizer.hpp"
#include "videoinput.hpp"
#include "window.hpp"

namespace app
{

class App
{
public:
	// Variables
	
	
	// Functions
	static App* getInstance();
	static void releaseInstance();
	bool initialize();
	bool run();
	
	
private:
	App();
	~App();
	
	
	// Variables
	static App* instance;
	static bool useWebcam;
	static Window* window;
	static VideoInput* camera;
	static Recognizer* recognizer;
	
	
	// Functions
	void grabFrame();
	std::map<unsigned int, cv::Scalar> convertToCvColor(std::map<unsigned int, nanogui::Color> &ngColors);
	
};

}

#endif // APP_H
