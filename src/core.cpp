#include <iostream>
#include <cstdlib>
#include <dlfcn.h> 
#include <sys/inotify.h>
#include <unistd.h>

#include "gl_core_3_3.h"
#include "demo.h"
#include "sdlutil.h"

using namespace cv;

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX +1))

//demo functions
struct demo_app {
	void *demoLib;
	void (*initf)( demo_memory* );
	void (*updatef)( demo_memory*);
	void (*renderf)( demo_memory* );
	void (*updateshaderf)( demo_memory* );
};

//prototypes

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
	
	if (mem.permanentMemory == NULL) {
		fprintf(stderr, "Couldn't allocate memory");
		exit(EXIT_FAILURE);
	}

	//camera memory
	mem.videoCapMem = malloc(sizeof(VideoCapture));

	//set file watch
	inotifyFd = inotify_init1(IN_NONBLOCK);
	if (inotifyFd == -1) {
		fprintf(stderr, "Couldn't init inotify");
		exit(EXIT_FAILURE);
	}
	wd = inotify_add_watch(inotifyFd, "obj/", IN_CLOSE_WRITE);
	if (wd == -1) {
		fprintf(stderr, "Couldn't inotify_add_watch");
		exit(EXIT_FAILURE);
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
