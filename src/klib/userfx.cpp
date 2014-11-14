#include <ll/stdio.h>
#include <ll/stdlib.h>

#include "utente.h"

extern "C" void*ki_malloc(size_t);
FILE * stdout = (FILE*)1;
FILE *stderr = (FILE*)2;

extern "C"
int fprintf(FILE * fp,char *fmt,...) 
{
	char buf[1024];
	va_list va;
	va_start(va,fmt);
	vsprintf(buf, fmt, va);
	va_end(va);
	print(buf, fp == stderr ? 2: 0);
	return 0;
}

int putchar(int c)
{
	char cz[2] = {c,0};
	print(cz, 0);
	return c;
}

extern "C" void * malloc(int sz)
{
	fprintf(stdout, "Malloc %d\n", sz);
	return ki_malloc(sz);
}

