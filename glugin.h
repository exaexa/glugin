

#define MOZ_X11
#include "npapi.h"
#include "npupp.h"

typedef struct pluginData_t {
	//UNIX part
	Window win;
	pid_t child;
	FILE *pr,*pw;
	Display* dpy;

	int has_window;

	//common part
	void(*swapbuffers)(struct pluginData_t*);
	void(*getsize)(struct pluginData_t*,int*,int*); //TODO

	int exit_request,exit_ack;

	int r,g,b,br,bg,bb;
} pluginData;

extern "C"
void glugin_proc(pluginData* pd);

