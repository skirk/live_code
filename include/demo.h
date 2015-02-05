#ifndef _DEMO_H_
#define _DEMO_H_

#include "glslprogram.h"
#include <opencv2/opencv.hpp>

struct demo_memory {
	bool isInitialized;	
	int memorySize;
	void *permanentMemory;	
	void *videoCapMem;
};

struct demo_state {
	cv::VideoCapture *cap; //camera feed
	GLSLProgram shader;
	GLuint texture;
	GLuint vboHandles[3];
	GLuint vaoHandle;
	float time;
	int xcells;
	int ycells;
};

#endif
