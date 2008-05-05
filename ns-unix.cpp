
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

#include <GL/glx.h>

#include "glugin.h"

static NPNetscapeFuncs gNetscapeFuncs;	/* Netscape Function table */

void Log(const char* c,...)
{
	va_list al;
	FILE*f=fopen(".glugin.log","a");
	if(!f)return;
	va_start(al,c);
	vfprintf(f,c,al);
	va_end(al);
	fputc('\n',f);
	fclose(f);
};

NPError NP_Initialize(NPNetscapeFuncs * nsTable, NPPluginFuncs * pluginFuncs)
{
	Log("init");

	/* this is respectfully taken from mplayerplug-in, as it was observed
	 * and proved to be extremely useful. */
	if ((nsTable == NULL) || (pluginFuncs == NULL))
		return NPERR_INVALID_FUNCTABLE_ERROR;
	if ((nsTable->version >> 8) > NP_VERSION_MAJOR)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	if (nsTable->size < sizeof(NPNetscapeFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;
	if (pluginFuncs->size < sizeof(NPPluginFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;

	gNetscapeFuncs.version = nsTable->version;
	gNetscapeFuncs.size = nsTable->size;
	gNetscapeFuncs.posturl = nsTable->posturl;
	gNetscapeFuncs.geturl = nsTable->geturl;
	gNetscapeFuncs.requestread = nsTable->requestread;
	gNetscapeFuncs.newstream = nsTable->newstream;
	gNetscapeFuncs.write = nsTable->write;
	gNetscapeFuncs.destroystream = nsTable->destroystream;
	gNetscapeFuncs.status = nsTable->status;
	gNetscapeFuncs.uagent = nsTable->uagent;
	gNetscapeFuncs.memalloc = nsTable->memalloc;
	gNetscapeFuncs.memfree = nsTable->memfree;
	gNetscapeFuncs.memflush = nsTable->memflush;
	gNetscapeFuncs.reloadplugins = nsTable->reloadplugins;
	gNetscapeFuncs.getJavaEnv = nsTable->getJavaEnv;
	gNetscapeFuncs.getJavaPeer = nsTable->getJavaPeer;
	gNetscapeFuncs.getvalue = nsTable->getvalue;
	gNetscapeFuncs.setvalue = nsTable->setvalue;

	pluginFuncs->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
	pluginFuncs->size = sizeof(NPPluginFuncs);
	pluginFuncs->newp = NPP_New;
	pluginFuncs->destroy = NPP_Destroy;
	pluginFuncs->event = NPP_HandleEvent;
	pluginFuncs->setwindow = NPP_SetWindow;
	pluginFuncs->print = NPP_Print;
	pluginFuncs->newstream = NPP_NewStream;
	pluginFuncs->destroystream = NPP_DestroyStream;
	pluginFuncs->asfile = NPP_StreamAsFile;
	pluginFuncs->writeready = NPP_WriteReady;
	pluginFuncs->write = NPP_Write;

	return NPERR_NO_ERROR;
}

NPError NP_Shutdown() {
	Log("shutdown");
	return NPERR_NO_ERROR;
}


char* NP_GetMIMEDescription() {
	Log("get mime");
	return "x-application/glugin:gln:Glugin;";
}

NPError NP_GetValue(
	void*instance,
	NPPVariable variable,
	void* value)
{
	Log("get value");
	switch(variable) {
		case NPPVpluginNameString:
			(*(char**)value)="Glugin";
			break;
		case NPPVpluginDescriptionString:
			(*(char**)value)="the GL plugin";
			break;
		default:
			return NPERR_INVALID_PARAM;
	}
	return NPERR_NO_ERROR;
}

bool read_color(char* str, int*r, int*g, int*b)
{
	int len=strlen(str);
	if(!((len==3)||(len==6)))return false;

	int color;
	if(!sscanf(str,"%x",&color))return false;

	if(len==6) {
		*r=(color/65536)%256;
		*g=(color/256)%256;
		*b=color%256;
	} else {
		*r=255*((color/256)%16)/15;
		*g=255*((color/16)%16)/15;
		*b=255*(color%16)/15;
	}
	Log("Read color: %d %d %d\n",*r,*g,*b);
	return true;
}

NPError NPP_New(
	NPMIMEType mime,
	NPP instance,
	uint16 mode,
	int16 argc, char*argn[], char*argv[],
	NPSavedData* saved)
{
	int i;
	pluginData* pd;

	Log("new p");
	gNetscapeFuncs.setvalue(instance, NPPVpluginTransparentBool, false);


	instance->pdata=gNetscapeFuncs.memalloc(sizeof(pluginData));

	Log("allocated");
	
	pd=(pluginData*)(instance->pdata);
	pd->exit_request=pd->exit_ack=pd->has_window=
		pd->r=pd->g=pd->b=0;
	pd->br=pd->bg=pd->bb=255;

	for(i=0;i<argc;++i){ Log(argn[i]);Log(argv[i]);Log("--"); 
		if(!strcmp(argn[i],"gln-color")) {
			Log("AAA");
			read_color(argv[i],
				&(pd->r),
				&(pd->g),
				&(pd->b));}
		else if(!strcmp(argn[i],"gln-bgcolor")) {
			Log("BBB");
			read_color(argv[i],
				&(pd->br),
				&(pd->bg),
				&(pd->bb));}

	}

	return NPERR_NO_ERROR;
}

void set_nonblock(int fd)
{
	long flags;

	flags=fcntl(fd, F_GETFL, 0);
	flags|=O_NONBLOCK|O_NDELAY;
	fcntl(fd, F_SETFL, flags);
}

void plugin_process_handler(pluginData*, Window, Visual*, FILE*, FILE*);

NPError NPP_SetWindow(
	NPP instance,
	NPWindow *w)
{
	pluginData*pd=(pluginData*)(instance->pdata);
	pid_t child;
	int pw[2],pr[2];

	if(!w) {
		Log("setwindow: destroyed");
		return NPERR_NO_ERROR;
	}

	if(!pd->has_window){
		pd->has_window=1;

		pipe(pw);
		pipe(pr);

		set_nonblock(pr[0]);
		set_nonblock(pr[1]);
		set_nonblock(pw[0]);
		set_nonblock(pw[1]);

		child=fork();

		if(child==-1) return NPERR_MODULE_LOAD_FAILED_ERROR;

		if(!child) {
			Log("in new process!");

			close(pw[1]);
			close(pr[0]);

			//TODO check if setsid is needed here
			//
			plugin_process_handler(pd, pd->win=(Window)(w->window),
				((NPSetWindowCallbackStruct*)
					(w->ws_info))->visual,
				fdopen(pw[0],"r"),fdopen(pr[1],"w"));
			_exit(0);

		} else {
			pd->child=child;

			close(pw[0]);
			close(pr[1]);
			pd->pw=fdopen(pw[1],"w");
			pd->pr=fdopen(pr[0],"r");
		}
	}
	
	//we don't really care about window resize events, as long as
	//it can catch them by native methods

	return NPERR_NO_ERROR;
}


int16 NPP_HandleEvent(
	NPP instance,
	void* event)
{
	Log("event");

	return true;
}

NPError NPP_Destroy(
	NPP instance,
	NPSavedData** saved)
{
	pluginData* pd;
	pd=(pluginData*)(instance->pdata);

	Log("killing child...");

	kill(pd->child,SIGUSR1);
	usleep(10000);

	//TODO
	//wait until the child really terminates. Mutexes?

	gNetscapeFuncs.memfree(instance->pdata);
	Log("deleted ok");

	return NPERR_NO_ERROR;
}

void NPP_Print(NPP instance, NPPrint *printInfo)
{
	Log("attempt to print");
}

NPError NPP_NewStream(
	NPP instance, 
	NPMIMEType type,
	NPStream*  stream,
	NPBool     seekable,
	uint16*    stype)
{
	Log("new stream");
	return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream(
	NPP instance, 
	NPStream* stream, 
	NPReason reason)
{
	Log("destroy stream");
	return NPERR_NO_ERROR;
}

int32 NPP_Write(NPP instance, 
                NPStream* stream,
                int32 offset, 
                int32 len, 
                void* buf)
{
	Log("write %d",len);
	return len;
}

void NPP_StreamAsFile(
	NPP instance,
	NPStream* stream,
	const char* fname)
{
	Log("as file:");
	Log(fname);
}

int32 NPP_WriteReady(NPP instance, NPStream* stream)
{
	Log("write ready");
	return 1024; //ooh replace.
}

void resize(Display* dpy, Window win)
{
	XWindowAttributes wa;
	XGetWindowAttributes (dpy, win, &wa);
	glViewport(0,0,wa.width, wa.height);
}


static pluginData*g_pd;

void sig_usr1(int i) {
	g_pd->exit_request=1;
}

void swapbuffers(pluginData*pd)
{
	glXSwapBuffers(pd->dpy,pd->win);
}

void plugin_process_handler(pluginData*pd, Window win, Visual*vis, FILE*in, FILE*out)
{
	int i;
	Display*dpy=XOpenDisplay(0);
	XVisualInfo xvi;
	GLXContext ctx;

	g_pd=pd;

	signal(SIGUSR1, sig_usr1);

	Log("in plugin process...");
	xvi.visualid=vis->visualid;
	ctx=glXCreateContext(dpy,
		XGetVisualInfo(dpy,VisualIDMask,&(xvi),&i),
		0,GL_TRUE);
	glXMakeCurrent(dpy,win,ctx);
	
	if(glXIsDirect(dpy,ctx))
		Log("+++ Direct rendering");
	else	Log("!!! Indirect rendering");

	pd->swapbuffers=swapbuffers;
	pd->dpy=dpy;
	pd->win=win;

	glugin_proc(pd);

	Log("plugin killed, cleaning up.");
	
	glXMakeCurrent(dpy, None, 0);
	glXDestroyContext(dpy,ctx);
}

