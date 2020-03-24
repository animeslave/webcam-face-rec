#include "app.hpp"

namespace app
{

App* App::instance = nullptr;
GLFWwindow* App::window = nullptr;
dlib::frontal_face_detector App::faceDetector;
dlib::shape_predictor App::faceModel;
GLuint* App::VAO = nullptr;
GLuint* App::VBO = nullptr;
GLuint* App::EBO = nullptr;
GLuint* App::webcamTexture = nullptr;
GLuint* App::modelTexture = nullptr;
GLuint App::programID;
bool App::drawActualPoints = false;
bool App::drawCorrectedPoints = false;
GLuint App::DEFAULT_WIDTH = 800;
GLuint App::DEFAULT_HEIGHT = 600;

App::App(){}
App::~App(){}

// Is called whenever a key is pressed/released via GLFW
void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		if (drawActualPoints == false)
		{
			drawActualPoints = true;
		} else {
			drawActualPoints = false;
		}
	}
	if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
	{
		if (drawCorrectedPoints == false)
		{
			drawCorrectedPoints = true;
		} else {
			drawCorrectedPoints = false;
		}
	}
}

void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

App* App::getInstance() 
{
	if (instance == nullptr)
		instance = new App();
	return instance;
}

void App::releaseInstance()
{
	if (instance != nullptr)
		delete instance;
	instance = nullptr;
}

bool App::initialize() 
{
	Timer::getInstance()->start();

	// Initialize GLFW
	if(!glfwInit())
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}
	
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif 
	
	window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Face", NULL, NULL);
	if (window == NULL) {
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	
	VideoInput::getInstance()->openCamera();
	
	faceDetector = dlib::get_frontal_face_detector();
	dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> faceModel;
	
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		fprintf( stderr, "Failed to initialize OpenGL context.\n" );
		getchar();
		return -1;
	}
	
	glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "vertexshader", "fragmentshader" );
	
	static const GLfloat vertices[] = {
	// |     Position     ||  TexCoord  |
	   -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, // top left
		1.0f,  1.0f,  0.0f,  1.0f,  0.0f, // top right
	   -1.0f, -1.0f,  0.0f,  0.0f,  1.0f, // below left
		1.0f, -1.0f,  0.0f,  1.0f,  1.0f  // below right 
	};
	// Set up index
	static const GLuint indices[] = {
		0, 2, 1, 1, 2, 3
	};
	
	VAO = new GLuint();
	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);
	
	VBO = new GLuint();
	glGenBuffers(1, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	
	EBO = new GLuint();
	glGenBuffers(1, EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	webcamTexture = new GLuint();
	glGenTextures(1, webcamTexture);
	glBindTexture(GL_TEXTURE_2D, *webcamTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	modelTexture = new GLuint();
	glGenTextures(1, modelTexture);
	glBindTexture(GL_TEXTURE_2D, *modelTexture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// Kalman 
	
	
	return true;
}

bool App::run()
{
	char* titlestr = new char[255];
	while (!glfwWindowShouldClose(window)) 
	{
		sprintf( titlestr, "videoInput Demo App (%.1f ms)", Timer::getSpeedOnMS() );
		glfwSetWindowTitle(window, titlestr);
		
		cv::Mat tframe, frame,
			frameRGB, frameRGBBilateraled, frameRGBResized, frameRGBBluredResized,
			frameGray, frameGrayBlured, frameGrayResized, frameGrayBluredResized, frameGrayEqualized, 
			frameCannied, frameBilateraled;
		
		// Grab a frame
		
		VideoInput::getInstance()->grabFrame(&tframe);
		cv::flip(tframe, tframe, 1);
		
		cv::cvtColor(tframe, frameRGB, cv::COLOR_BGR2RGB);
		cv::resize(frameRGB, frameRGBResized, cv::Size(), 0.5, 0.5);
		cv::GaussianBlur(frameRGBResized, frameRGBBluredResized, cv::Size( 5, 5 ), 0, 0 );
		
		cv::cvtColor(tframe, frameGray, cv::COLOR_BGR2GRAY);
		cv::resize(frameGray, frameGrayResized, cv::Size(), 0.5, 0.5);
		cv::GaussianBlur(frameGrayResized, frameGrayBluredResized, cv::Size( 5, 5 ), 0, 0 );
				
		//
		cv::equalizeHist(frameGray, frameGrayEqualized);
		cv::Canny(tframe, frameCannied, 10, 20, 7, true);
		//cv::erode(frame, tleftEye, kernel);
		
		frame = frameGrayEqualized;
		
		IplImage imgRGB = cvIplImage(frameRGBBluredResized);
		IplImage imgGray = cvIplImage(frameGrayBluredResized);
		
		dlib::cv_image<dlib::rgb_pixel> cimgRGB(&imgRGB);
		dlib::cv_image<unsigned char> cimgGray(&imgGray);
		
		dlibDetectAndDraw(frame, cimgRGB, cv::Scalar(255,255,0));
		dlibDetectAndDraw(frame, cimgGray, cv::Scalar(127,127,127));
		
		
		
		glfwMakeContextCurrent(window);
		glBindTexture(GL_TEXTURE_2D, *webcamTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, frame.cols, frame.rows, 0, GL_RED, GL_UNSIGNED_BYTE, frame.data);
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(programID);
		glBindVertexArray(*VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		
		glfwSwapBuffers(window);
		
		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
	}
	
	// Cleanup VBO
	glDeleteBuffers(1, VBO);
	glDeleteBuffers(1, EBO);
	glDeleteVertexArrays(1, VAO);
	glDeleteProgram(programID);
	
	// Release OpenCV webcam capture
	VideoInput::getInstance()->releaseCamera();
	
	// Terminates GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return true;
}



template < typename T >
void App::dlibDetectAndDraw(cv::Mat frame, T cimg, cv::Scalar color)
{
	
	// Detect faces 
	std::vector<dlib::rectangle> faces = faceDetector(cimg);
	
	// Find the pose of each face.
	std::vector<dlib::full_object_detection> faceShapes;
	
	for (unsigned long i = 0; i < faces.size(); ++i)
	{
		dlib::full_object_detection f = faceModel(cimg, faces[i]);
		faceShapes.push_back(f);
	}
	
//	if (!shapes.empty())
//	{
//		// Simple visualization of face detection
//		//for (int i = 0; i < 68; i++)
//		//{
//		//	cv::circle(frame, cvPoint(shapes[0].part(i).x()*2, shapes[0].part(i).y()*2), 2, cv::Scalar(255, 255, 255), -1);
//		//	cv::circle(frame, correctedPosition[i], 2, cv::Scalar(255, 0, 0), -1);
//		//}
//		
//		// Advanced visualization of face detection
//		// Cheek to cheek shape (parts 0-16)
//		for (int i = 0; i < 16; i++)
//		{
//			if (drawActualPoints == true)
//				cv::line(frame, actualPosition[i], actualPosition[i+1], color, 1);
//			if (drawCorrectedPoints == true)
//				cv::line(frame, correctedPosition[i], correctedPosition[i+1], color, 1);
//		}
//		
//		// Right eyebrow shape (parts 17-21)
//		for (int i = 17; i < 21; i++)
//		{
//			if (drawActualPoints == true)
//				cv::line(frame, actualPosition[i], actualPosition[i+1], color, 1);
//			if (drawCorrectedPoints == true)
//				cv::line(frame, correctedPosition[i], correctedPosition[i+1], color, 1);
//		}
//		
//		// Left eyebrow shape (parts 22-26)
//		for (int i = 22; i < 26; i++)
//		{
//			if (drawActualPoints == true)
//				cv::line(frame, actualPosition[i], actualPosition[i+1], color, 1);
//			if (drawCorrectedPoints == true)
//				cv::line(frame, correctedPosition[i], correctedPosition[i+1], color, 1);
//		}
//		
//		// Nose shape (parts 27-35)
//		for (int i = 27; i < 35; i++)
//		{
//			if (drawActualPoints == true)
//				cv::line(frame, actualPosition[i], actualPosition[i+1], color, 1);
//			if (drawCorrectedPoints == true)
//				cv::line(frame, correctedPosition[i], correctedPosition[i+1], color, 1);
//		}
//		
//		cv::putText(
//				frame, 
//				std::to_string(shapes[0].part(35).x()*2 - shapes[0].part(31).x()*2), 
//				cvPoint(shapes[0].part(33).x()*2, shapes[0].part(33).y()*2), 
//				cv::FONT_HERSHEY_SIMPLEX, 0.4, color, 1);
//		
//		// Right eye shape (parts 36-41)
//		long int rightEye_x_min = actualPosition[36].x;
//		long int rightEye_x_max = actualPosition[41].x;
//		long int rightEye_y_min = actualPosition[36].y;
//		long int rightEye_y_max = actualPosition[41].y;
//		for (int i = 36; i < 41; i++)
//		{
//			if ( i == 40) 
//			{
//				if (drawActualPoints == true)
//					cv::line(frame, actualPosition[i], actualPosition[36], color, 1);
//				if (drawCorrectedPoints == true)
//					cv::line(frame, correctedPosition[i], correctedPosition[36], color, 1);
//			} else {
//				if (drawActualPoints == true)
//					cv::line(frame, actualPosition[i], actualPosition[i+1], color, 1);
//				if (drawCorrectedPoints == true)
//					cv::line(frame, correctedPosition[i], correctedPosition[i+1], color, 1);
//			}
//			
//			// Found min and max of x and y for locate right eye
//			if (actualPosition[i].x < rightEye_x_min)
//				rightEye_x_min = actualPosition[i].x;
//			if (actualPosition[i].x > rightEye_x_max)
//				rightEye_x_max = actualPosition[i].x;
//			if (actualPosition[i].y < rightEye_y_min)
//				rightEye_y_min = actualPosition[i].y;
//			if (actualPosition[i].y > rightEye_y_max)
//				rightEye_y_max = actualPosition[i].y;
//		}
//		
//		// Left eye shape (parts 42-47)
//		long int leftEye_x_min = actualPosition[42].x; 
//		long int leftEye_x_max = actualPosition[47].x; 
//		long int leftEye_y_min = actualPosition[42].y; 
//		long int leftEye_y_max = actualPosition[47].y;
//		for (int i = 42; i < 47; i++)
//		{
//			if ( i == 46) 
//			{
//				if (drawActualPoints == true)
//					cv::line(frame, actualPosition[i], actualPosition[42], color, 1);
//				if (drawCorrectedPoints == true)
//					cv::line(frame, correctedPosition[i], correctedPosition[42], color, 1);
//			} else {
//				if (drawActualPoints == true)
//					cv::line(frame, actualPosition[i], actualPosition[i+1], color, 1);
//				if (drawCorrectedPoints == true)
//					cv::line(frame, correctedPosition[i], correctedPosition[i+1], color, 1);
//			}
//			if (actualPosition[i].x < leftEye_x_min)
//				leftEye_x_min = actualPosition[i].x;
//			if (actualPosition[i].x > leftEye_x_max)
//				leftEye_x_max = actualPosition[i].x;
//			if (actualPosition[i].y < leftEye_y_min)
//				leftEye_y_min = actualPosition[i].y;
//			if (actualPosition[i].y > leftEye_y_max)
//				leftEye_y_max = actualPosition[i].y;
//		}
//		
//		// Outer mouth shape (parts 48-59)
//		for (int i = 48; i < 59; i++)
//		{
//			if ( i == 58) 
//			{
//				if (drawActualPoints == true)
//					cv::line(frame, actualPosition[i], actualPosition[48], color, 1);
//				if (drawCorrectedPoints == true)
//					cv::line(frame, correctedPosition[i], correctedPosition[48], color, 1);
//			} else {
//				if (drawActualPoints == true)
//					cv::line(frame, actualPosition[i], actualPosition[i+1], color, 1);
//				if (drawCorrectedPoints == true)
//					cv::line(frame, correctedPosition[i], correctedPosition[i+1], color, 1);
//			}
//		}
//		
//		// Inner mouth shape (parts 60-67)
//		for (int i = 60; i < 68; i++)
//		{
//			if (i == 67) 
//			{
//				if (drawActualPoints == true)
//					cv::line(frame, actualPosition[i], actualPosition[60], color, 1);
//				if (drawCorrectedPoints == true)
//					cv::line(frame, correctedPosition[i], correctedPosition[60], color, 1);
//			} else {
//				if (drawActualPoints == true)
//					cv::line(frame, actualPosition[i], actualPosition[i+1], color, 1);
//				if (drawCorrectedPoints == true)
//					cv::line(frame, correctedPosition[i], correctedPosition[i+1], color, 1);
//			}
//		}
//	}
}

}

