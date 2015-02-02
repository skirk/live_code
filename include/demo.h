#ifndef _DEMO_H_
#define _DEMO_H_
#include "glslprogram.h"
#include <opencv2/opencv.hpp>

using namespace cv;

struct demo_memory {
	bool isInitialized;	
	int memorySize;
	void *permanentMemory;	
	void *videoCapMem;
};

struct demo_state {
	GLSLProgram shader;
	VideoCapture *cap; // open the default camera
	GLuint texture;
	GLuint vboHandles[3];
	GLuint vaoHandle;
	float time;
	int xcells;
	int ycells;
};


#endif
