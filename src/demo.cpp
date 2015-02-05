#include "gl_core_3_3.h"
#include "demo.h"

using namespace std;
using namespace cv;

//prototypes
static int loadShader(const string &myBinaryFileName, GLSLProgram *prog);
static int createTexture(GLuint *text);


struct vec2{
	float x;
	float y;
};


struct cell {
	//corners
	vec2 tlc;	
	vec2 blc;	
	vec2 trc;	
	vec2 brc;	
};

void createCells(int xcells, int ycells, cell *cells, vec2 xrange, vec2 yrange){
	
	float xlength = xrange.y - xrange.x;
	float ylength = yrange.y - yrange.x;
	float xshift  = xlength/(float)xcells;
	float yshift  = ylength/(float)ycells;

	for (int y = 0; y < ycells; y++) {
		for (int x = 0; x < xcells; x++) {
			int i = y*xcells +x;
			cells[i] = {
		       	 {xrange.x +    x * xshift, yrange.x +    y * yshift},
			 {xrange.x +    x * xshift, yrange.x + (y+1)* yshift},
			 {xrange.x + (x+1)* xshift, yrange.x +    y * yshift},
		       	 {xrange.x + (x+1)* xshift, yrange.x + (y+1)* yshift},
			};
		}
	}
}



void createIndices(int ncells, int *indices) {
	for (int i = 0; i < ncells; i++) { 
		//a square has 4 corners
		int cellinds [6]= {i*4, i*4+1, i*4+2, i*4+1, i*4+2, i*4+3};
		memcpy(indices + i * 6, cellinds, sizeof(cellinds));
	}
}

extern "C" void initdemo( demo_memory *memory ) {

	demo_state *state = (demo_state*) memory->permanentMemory;

	if (!memory->isInitialized) {

		state->time = 0;
		state->cap = new(memory->videoCapMem) VideoCapture(0);
		if (!state->cap->isOpened() ) 
			perror("camera failed to open\n");

		createTexture(&state->texture);

		memory->isInitialized = true;
	}

	state->xcells=40;
	state->ycells=40;

	int indices[state->xcells*state->ycells*6];
	cell positiondata[state->xcells*state->ycells];
	cell uvdata[state->xcells*state->ycells];
	createCells(state->xcells, state->ycells, positiondata, {-1,1}, {-1,1});
	createCells(state->xcells, state->ycells, uvdata, {0,1}, {0,1});
	createIndices(state->xcells*state->ycells, indices);

	glBindAttribLocation(state->shader.getHandle(), 0, "VertexPosition");
	glBindAttribLocation(state->shader.getHandle(), 1, "TexCoord");

	//generate buffers
	glGenBuffers(3, state->vboHandles);
	glGenVertexArrays( 1, &state->vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, state->vboHandles[0]);
	glBufferData(GL_ARRAY_BUFFER, 
			sizeof(positiondata), positiondata, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, state->vboHandles[1]);
	glBufferData(GL_ARRAY_BUFFER, 
			sizeof(uvdata), uvdata, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->vboHandles[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
			sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(state->vaoHandle);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, state->vboHandles[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, state->vboHandles[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->vboHandles[2]);

	//glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

extern "C" void render( demo_memory *memory) {

	demo_state *state = (demo_state*) memory->permanentMemory;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawElements(GL_TRIANGLES_ADJACENCY,
		       	state->xcells*state->ycells*6,GL_UNSIGNED_INT, NULL );

}

extern "C" void update( demo_memory *memory ) {

	demo_state *state = (demo_state*) memory->permanentMemory;
	state->time += 0.1;
	state->shader.setUniform("time", state->time);
	Mat frame;
	*(state->cap) >> frame; // get a new frame from camera
	flip(frame, frame, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			frame.cols, frame.rows,
			0, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
}

extern "C" void updateshader( demo_memory *memory) {

	demo_state *state = (demo_state*) memory->permanentMemory;
	GLSLProgram temp_prog;
	if (loadShader("obj/shader.bin", &temp_prog) == EXIT_FAILURE) {
		return;
	} else { 
		state->shader = temp_prog;
		state->shader.use();
	}
}

int createTexture(GLuint *text) {

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, text);

	glBindTexture(GL_TEXTURE_2D, *text);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);

	return 0;
}

int loadShader(const string &myBinaryFileName, GLSLProgram *prog) {
	GLint   binaryLength;
	void*   binary;
	GLint   success;
	FILE*   infile;

	if (prog->getHandle() < 0)
		prog->setHandle( glCreateProgram() );

	infile = fopen(myBinaryFileName.c_str(), "rb");
	fseek(infile, 0, SEEK_END);
	binaryLength = (GLint)ftell(infile);
	binary = (void*)malloc(binaryLength);
	fseek(infile, 0, SEEK_SET);
	fread(binary, binaryLength, 1, infile);
	fclose(infile);

	int nformats;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &nformats);
	GLint formats[nformats];
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats);

	glProgramBinary(prog->getHandle(), formats[0], binary, binaryLength);
	free(binary);

	glGetProgramiv(prog->getHandle(), GL_LINK_STATUS, &success);
	if (!success)
	{
		fprintf(stderr, "Couldn't open shader");
		return EXIT_FAILURE;
	}
	glUseProgram(prog->getHandle());

	return EXIT_SUCCESS;
}
