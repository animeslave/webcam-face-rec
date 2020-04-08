#ifndef APP_H
#define APP_H

// Live2D


// OpenCV
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// CMUSphinx
//#include <pocketsphinx/pocketsphinx.h>

// Inner classes
#include "faceshapes.hpp"
#include "opengl.hpp"
#include "timer.hpp"
#include "videoinput.hpp"
#include "window.hpp"

namespace app
{

class App
{
public:
	
	// Functions
	static App* getInstance();
	static void releaseInstance();
	bool initialize();
	bool run();
	
	// Variables
	
	
private:
	App();
	~App();
	
	// Functions
	
	
	// Variables
	static App* instance;
	static bool bUseWebcam;
	
	
	
	
	
};

}

#endif // APP_H
