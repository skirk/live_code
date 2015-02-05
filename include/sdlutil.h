#ifndef _SDLUTIL_H_
#define _SDLUTIL_H_

#include <SDL2/SDL.h>

//sdl context creation functions
SDL_Window *createWindow( const char* title,
  	int position_x, int position_y, 
  	int width, int height,
  	Uint32 flags);
SDL_GLContext createContext (SDL_Window*);
void deleteContext (SDL_GLContext);

#endif 
