
#include <stdio.h>
#include <stdarg.h>

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
}

