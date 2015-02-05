#include "demo.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h> 
#include <cstdlib>
#include <dlfcn.h> 
#include <sys/inotify.h>
#include <unistd.h>

#include "gl_core_3_3.h"


#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX +1))

struct demo_app {
	
	demo_app & operator= (const demo_app &c) {
		       	demoLib= c.demoLib;
		       	initf = c.initf;
		       	updatef = c.updatef;
		       	renderf = c.renderf;
		       	updateshaderf = c.updateshaderf;
			return *this;
	};
	void *demoLib;
	void (*initf)( demo_memory* );
	void (*updatef)( demo_memory*);
	void (*renderf)( demo_memory* );
	void (*updateshaderf)( demo_memory* );
};

//prototypes
static SDL_Window *createWindow( const char* title,
		int position_x, int position_y, 
		int width, int height,
		Uint32 flags);
static SDL_GLContext createContext (SDL_Window*);
static void deleteContext (SDL_GLContext);
static void mainloop(SDL_Window *_win);
static int loadDemo(demo_app *demo, char *demopath);
static void closeDemo(demo_app demo);



int main() {

	SDL_Window *win = createWindow("SDL Project", 0, 0, 512, 512,  SDL_WINDOW_OPENGL);
	SDL_GLContext context = createContext(win);

	mainloop(win);

	deleteContext(context);

	return EXIT_SUCCESS;
}

void mainloop (SDL_Window *_win) {

	char demopath[] = "obj/libdemo.so";
	int inotifyFd, wd;
	struct inotify_event *ievent;
	char buf[BUF_LEN];
	int quit = 0;
	ssize_t numRead;
	char *p;

	//load demo
	demo_app demo;
	loadDemo(&demo, demopath);


	//allocate total memory for the demo
	//no further allocations from the heap
	demo_memory mem = {};
	mem.memorySize = Megabytes (5);

	mem.permanentMemory = calloc(mem.memorySize, sizeof(char));
	if (mem.permanentMemory == NULL)
		perror("Couldn't allocate memory");
	mem.videoCapMem = malloc(sizeof(VideoCapture));

	//set file watch
	inotifyFd = inotify_init1(IN_NONBLOCK);
	if (inotifyFd == -1) {
		perror("Couldn't init inotify");
		exit(EXIT_FAILURE)
	}
	wd = inotify_add_watch(inotifyFd, "obj/", IN_CLOSE_WRITE);
	if (wd == -1) {
		perror("Couldn't inotify_add_watch");
		exit(EXIT_FAILURE)
	}


	(*demo.updateshaderf)(&mem);
	(*demo.initf)(&mem);


	while(!quit) {

		SDL_Event event;
		//dynamically load a library when new version is present
		numRead = read(inotifyFd, buf, BUF_LEN);

		for (p = buf; p < buf + numRead; ) {
			ievent = (struct inotify_event *) p;
			//displayInotifyEvent(ievent);
			if (ievent->mask & IN_CLOSE_WRITE) {
				//load new shader
				if (!strcmp(ievent->name, "shader.bin")) {
					(*demo.updateshaderf)(&mem);
					(*demo.initf)(&mem);
				//load new demo
				} else if (!strcmp(ievent->name, "libdemo.so")){
					demo_app demo_temp;
					closeDemo(demo);
					while (loadDemo(&demo_temp, demopath) < 0); 
					demo = demo_temp;
					(*demo.initf)(&mem);
				}
			}
			p += sizeof(struct inotify_event) + ievent->len;
		}

		while(SDL_PollEvent(&event) == 1) {
			switch(event.type) {
				case SDL_QUIT:
					std::cout<<"quit\n";
					quit = 1;
					break;
				default:
					break;
			}
		}
		(*demo.updatef)(&mem);
		(*demo.renderf)(&mem);
		SDL_GL_SwapWindow(_win);
	}
	SDL_Quit();
}

SDL_Window *createWindow(const char* title, int position_x, int position_y,
		int width, int height, Uint32 flags) {

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		std::cout << "SDL_Init Error: "<< SDL_GetError() << std::endl;
	}

	SDL_Window *win; // window to hold our context
	win = SDL_CreateWindow(title, position_x, position_y, width, height, flags); 
	if (win == NULL) { 
		std::cout<< "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
	}

	return win;
}

SDL_GLContext createContext(SDL_Window *win) {

	SDL_GL_SetSwapInterval(1);

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, 
			SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_GLContext context = SDL_GL_CreateContext( win );

	if (ogl_LoadFunctions() == ogl_LOAD_FAILED) 
		perror("OGL functions failed to load\n");


	return context;
}

void deleteContext(SDL_GLContext ctx) {
	SDL_GL_DeleteContext(ctx);
}

int loadDemo(demo_app *demo, char *demopath) { 

	const char *err;

	(void) dlerror(); // clear error

	demo->demoLib = dlopen (demopath, RTLD_LAZY);
	if (demo->demoLib == NULL) {
		return -1;
	}
	// load init, update and render
	*(void **)(&demo->updatef) = dlsym(demo->demoLib, "update");
	err = dlerror();
	if (err != NULL) {
		std::cerr<<"dlsym, update: "<<err<<"\n";
		return -1;
	}
	*(void **)(&demo->initf) = dlsym(demo->demoLib, "initdemo");
	err = dlerror();
	if (err != NULL) {
		std::cerr<<"dlsym, init: "<<err<<"\n";
		return -1;
	}
	*(void **)(&demo->renderf) = dlsym(demo->demoLib, "render");
	err = dlerror();
	if (err != NULL) {
		std::cerr<<"dlsym, render: "<<err<<"\n";
		return -1;
	}
	*(void **)(&demo->updateshaderf) = dlsym(demo->demoLib, "updateshader");
	err = dlerror();
	if (err != NULL) {
		std::cerr<<"dlsym, updateshaderf: "<<err<<"\n";
		return -1;
	}

	return 1;
}

void closeDemo(demo_app demo) {
	dlclose(demo.demoLib);
}
