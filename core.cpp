
#include "glugin.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <unistd.h>
#include <math.h>

void guest_read_host_requests(pluginData* pd)
{
	int cmd;
	NPStream* s;
	if(fread(&cmd,sizeof(int),1,pd->pr)<=0) return; //no data
	switch(cmd) {
		case gln_new_stream:
			fread(&s,sizeof(s),1,pd->pr);
			pd->handle_newstream(pd,s);
			break;
		case gln_destroy_stream:
			fread(&s,sizeof(s),1,pd->pr);
			pd->handle_destroystream(pd,s);
			break;
		case gln_stream_data:
			{
				int32 off,len;
				void*data;
				fread(&s,sizeof(s),1,pd->pr);
				fread(&off,sizeof(off),1,pd->pr);
				fread(&len,sizeof(len),1,pd->pr);
				if(len>0) {
					data=malloc(len);
					fread(data,len,1,pd->pr);
					pd->handle_write(pd,s,off,len,data);
					free(data);
				} else Log("w: zero len!");
			}
			break;
		default: Log("Unhandled request to guest: %d",cmd);
	}
}

void guest_newstream(pluginData*pd, NPStream*s)
{
	Log("guest has NEWSTREAM: %p",s);
}

void guest_destroystream(pluginData*pd, NPStream*s)
{
	Log("guest has DESTROYSTREAM: %p",s);
}

void guest_write(pluginData*pd,NPStream*s,int32 off,int32 len,void*data)
{
	Log("guest has WRITE: %p %d %d, data followz:",s,off,len);
	for(int i=0;i<len;++i)Log("%8X: %c",i,((char*)data)[i]);
}

void glugin_proc(pluginData*pd)
{
	float rot=0;
	float r=pd->r/255.0,g=pd->g/255.0,b=pd->b/255.0;

	float br=pd->br/255.0,bg=pd->bg/255.0,bb=pd->bb/255.0;

	pd->handle_newstream=guest_newstream;
	pd->handle_destroystream=guest_destroystream;
	pd->handle_write=guest_write;


	glClearColor(br,bg,bb,0);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80,1.3333,0.1,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	while(!(pd->exit_request)) {
		guest_read_host_requests(pd);
		usleep(1000);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(0,0,-2);
		glRotatef(rot,0,0,-1);
		{float ri,rj,gi,gj,bi,bj,tmp;
		ri=r;
		gi=g;
		bi=b;
		rj=(br-r)/100;
		gj=(bg-g)/100;
		bj=(bb-b)/100;
		glBegin(GL_TRIANGLE_FAN);
		glColor3f(br,bg,bb);
		glVertex2f(0,0);
		glVertex2f(0,1);
		for(int i=2;i<102;++i) {
			glColor3f(ri,gi,bi);
			tmp=M_PI*(3*i)/180.0;
			glVertex2f(-sinf(tmp),cosf(tmp));
			ri+=rj;
			gi+=gj;
			bi+=bj;
		}
		glEnd();
		}
		pd->swapbuffers(pd);

		rot+=0.5;
	}
	pd->exit_ack=1;
}
