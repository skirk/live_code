#include <SDL2/SDL_opengl.h> 
#include "sdlutil.h"


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


