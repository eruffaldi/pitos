// PMBIT.CPP
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#include "sistemag.h"
#include "utente.h"
#include "mypc.h"
#include "mylib.h"

static char buff[12000];

void umain(int h)
{
	char * cp = (char*)malloc(10);
	char * cp2 = (char*)malloc(100);
	fprintf(stderr,"message to system logging\n");
	fprintf(stdout,"message to Application Viewport\nTwo Lines\nAllocated Blocks %x %x\n",(int)cp,(int)cp2);
	free(cp);
	fprintf(stdout,"Block Freed\n");

	while(1);
}


extern "C"
void cmain()
{
	bool risu;
	int proc;

	kernel_init();
	log_message("Welcome into Message Logging for OSSYS");

	ki_activate_p(umain,0,0,proc, risu);
	ki_activate_p(umain,100,0,proc, risu);
	
	if(risu == false) 
		log_message("Non posso Allocare un Processo");

	kernel_go();
	
}

