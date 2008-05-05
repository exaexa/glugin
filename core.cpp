
#include "glugin.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <unistd.h>


void glugin_proc(pluginData*pd)
{
	float rot=0;
	float r=pd->r/255.0,g=pd->g/255.0,b=pd->b/255.0;

	float br=pd->br/255.0,bg=pd->bg/255.0,bb=pd->bb/255.0;

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
		usleep(1000);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(0,0,-2);
		glRotatef(rot,0,0,1);
		{float ri,rj,gi,gj,bi,bj;
		ri=r;
		gi=g;
		bi=b;
		rj=(br-r)/10;
		gj=(bg-g)/10;
		bj=(bb-b)/10;
		for(int i=0;i<10;++i) {
			glBegin(GL_TRIANGLES);
				glColor3f(ri,gi,bi);
				glVertex2f(0,0.3);
				glVertex2f(0.1,1);
				glVertex2f(-0.1,1);
			glEnd();
			ri+=rj;
			gi+=gj;
			bi+=bj;
			glRotatef(-20,0,0,1);
		}}
		pd->swapbuffers(pd);
		rot+=0.5;
	}
	pd->exit_ack=1;
}
