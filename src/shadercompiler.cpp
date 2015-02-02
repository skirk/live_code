#include <fstream>
#include <sstream>

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <cstdlib>

// OS specific stuff
#include <sys/stat.h>
#include <libgen.h>
#include <string>
#include "gl_core_3_3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

using namespace std;
SDL_Window *createWindow( const char* title,
		int position_x, int position_y, 
		int width, int height,
		Uint32 flags);
SDL_GLContext createContext (SDL_Window*);
void deleteContext (SDL_GLContext);



namespace GLSLShader {
	enum GLSLShaderType {
		VERTEX, FRAGMENT, GEOMETRY
	};
};


bool fileExists( const string & fileName );
GLuint compileShaderFromFile( const char * fileName, GLSLShader::GLSLShaderType type );
GLuint  compileShaderFromString( const string & source, GLSLShader::GLSLShaderType type );

int link(GLuint programhandle)
{
	glLinkProgram(programhandle);

	int status = 0;
	glGetProgramiv( programhandle, GL_LINK_STATUS, &status);
	if( GL_FALSE == status ) {
		int length = 0;
		char *logString;

		glGetProgramiv(programhandle, GL_INFO_LOG_LENGTH, &length );

		if( length > 0 ) {
			char * c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(programhandle, length, &written, c_log);
			logString = c_log;
			fprintf(stderr, "%s", logString);
			delete [] c_log;
		}

		return EXIT_FAILURE;
	} 
	return EXIT_SUCCESS;
}




GLuint compileShaderFromFile( const char * fileName,
		GLSLShader::GLSLShaderType type )
{
	if( ! fileExists(fileName) )
	{
		fprintf(stderr, "FILE not found :");
		return -EXIT_FAILURE;
	}

	ifstream inFile( fileName, ios::in );
	if( !inFile ) {
		fprintf(stderr, "Failed to open file");
		return -EXIT_FAILURE;
	}

	ostringstream code;
	while( inFile.good() ) {
		int c = inFile.get();
		if( ! inFile.eof() ) code << (char) c;
	}
	inFile.close();

	return compileShaderFromString(code.str(), type);
}

GLuint  compileShaderFromString( const string & source, GLSLShader::GLSLShaderType type )
{
	GLuint shaderHandle = 0;

	switch( type ) {
		case GLSLShader::VERTEX:
			shaderHandle = glCreateShader(GL_VERTEX_SHADER);
			break;
		case GLSLShader::FRAGMENT:
			shaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		case GLSLShader::GEOMETRY:
			shaderHandle = glCreateShader(GL_GEOMETRY_SHADER);
			break;
		default:
			fprintf(stderr, "No specified shader type availabl");
	}

	const char * c_code = source.c_str();
	glShaderSource( shaderHandle, 1, &c_code, NULL );

	// Compile the shader
	glCompileShader(shaderHandle);

	// Check for errors
	int result;
	glGetShaderiv( shaderHandle, GL_COMPILE_STATUS, &result );
	if( GL_FALSE == result ) {
		// Compile failed, store log and return false
		int length = 0;
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length );
		if( length > 0 ) {
			char * c_log = new char[length];
			int written = 0;
			glGetShaderInfoLog(shaderHandle, length, &written, c_log);
			fprintf(stderr, "%s", c_log);	
			delete [] c_log;
		}
		return -1;
	} else {
		// Compile succeeded, attach shader and return true
		return shaderHandle;
	}
}


/*
   void compileAndLinkShader(GLSLProgram *prog, char *vertex, char *fragment) {

   if (! prog->compileShaderFromFile(vertex,GLSLShader::VERTEX))
   {
   printf("Vertex shader failed to compile!\n%s",
   prog->log().c_str());
   exit(1);
   }
   if( ! prog->compileShaderFromFile(fragment ,GLSLShader::FRAGMENT))
   {
   printf("Fragment shader failed to compile!\n%s",
   prog->log().c_str());
   exit(1);
   }
   if( ! prog->link() )
   {
   printf("Shader program failed to link!\n%s",
   prog->log().c_str());
   exit(1);
   }

   prog->use();
   int numShaders;
   glGetAttachedShaders(prog, 3,
   }
   */

bool fileExists( const string & fileName )
{
	struct stat info;
	int ret = -1;

	ret = stat(fileName.c_str(), &info);
	return 0 == ret;
}

#define printable(ch) (isprint((unsigned char) ch) ? ch : '#')

static void /* Print "usage" message and exit */
usageError(char *progName, const char *msg, int opt)
{
	if (msg != NULL && opt != 0)
		fprintf(stderr, "%s (-%c)\n", msg, printable(opt));
	fprintf(stderr, "Usage: %s [-p arg] [-x]\n", progName);
	exit(EXIT_FAILURE);
}

struct shaderData {
	GLSLShader::GLSLShaderType type;
	char *file;
};

int main(int argc, char* argv[]) {


	shaderData shaders[2];
	int opt, i = 0;
	extern char *optarg;
	char *binaryTarget;
	while ((opt = getopt(argc, argv, ":v:f:g:o:")) != -1) {
		printf("\n");
		switch (opt) {
			case 'v': shaders [i++] = {GLSLShader::VERTEX,   optarg};  break;
			case 'f': shaders [i++] = {GLSLShader::FRAGMENT, optarg};  break;
			case 'g': shaders [i++] = {GLSLShader::GEOMETRY, optarg};  break;
			case 'o': binaryTarget = optarg; break;
			case ':': usageError(argv[0], "Missing argument", optopt);
			case '?': usageError(argv[0], "Unrecognized option", optopt);
			default: perror("Unexpected case in switch()");

		}
	}

	SDL_Window *win = createWindow("shader", 0, 0, 512, 512,  SDL_WINDOW_OPENGL);
	SDL_GLContext context = createContext(win);

	if(ogl_LoadFunctions() == ogl_LOAD_FAILED) {
		fprintf(stderr, "OGL functions failed to load\n");
		return EXIT_FAILURE;
	}
	GLuint handle = glCreateProgram();
	// PROGRAM_BINARY_RETRIEVABLE_HINT 
	glProgramParameteri(handle,  0x8257 , GL_TRUE);
	if (handle == 0)  {
		fprintf(stderr, "Unable to create a shader program\n");
		return EXIT_FAILURE;
	}

	for (int j=0; j<i; j++) {
		GLuint shader = compileShaderFromFile( shaders[j].file, shaders[j].type);
		if (shader > 0 ) {
			glAttachShader(handle, shader);
		} else {
			return EXIT_FAILURE;
		}
	}
	if (link(handle) == EXIT_FAILURE) {
		fprintf(stderr, "Linking Program failed\n");
		return EXIT_FAILURE;
	}

	int nformats;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &nformats);
	GLint formats[nformats];
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats);

	int binaryLength;
	void* binary;
	glGetProgramiv(handle, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
	binary = (void*)malloc(binaryLength);
	glGetProgramBinary(handle, binaryLength, NULL, (GLenum*)&formats[0], binary);

	FILE*   outfile;
	if ( (outfile = fopen(binaryTarget, "wb")) == NULL ) {
		fprintf(stderr, "Couldn't open file%s\n", binaryTarget);
		return EXIT_FAILURE;
	}
	fwrite(binary, binaryLength, 1, outfile);
	fclose(outfile);
	free(binary);



	deleteContext(context);
	return EXIT_SUCCESS;
}

SDL_Window *createWindow(const char* title, int position_x, int position_y, int width, int height, Uint32 flags) {

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		fprintf(stderr, "SDL_Init Error: %s", SDL_GetError());
	}

	SDL_Window *win; // window to hold our context
	win = SDL_CreateWindow(title, position_x, position_y, width, height, flags); 
	if (win == NULL) { 
		fprintf(stderr, "SDL_CreateWindow Error: %s ", SDL_GetError());
	}

	return win;
}

SDL_GLContext createContext(SDL_Window *win) {

	SDL_GL_SetSwapInterval(1);

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_GLContext context = SDL_GL_CreateContext( win );


	return context;
}

void deleteContext(SDL_GLContext ctx) {
	SDL_GL_DeleteContext(ctx);
}


