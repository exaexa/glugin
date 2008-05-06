

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


//op number, unsigned int
#define gln_new_stream 1 //stream *
#define gln_stream_data 2 //stream *, data offset, data size, data
#define gln_destroy_stream 3 //stream *
#define gln_request_get 4 //url size, url
#define gln_request_post 5 //url size, data size, url, data

void host_newstream(pluginData*, NPStream*);
void host_destroystream(pluginData*, NPStream*);
void host_write(pluginData*,NPStream*,int32, int32, void*);
void host_read_guest_requests(pluginData*);

