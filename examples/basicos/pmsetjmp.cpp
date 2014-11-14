// PMSETJMP.CPP
// Testing del Set/Long Jmp dentro ad un Processo
//
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "mypc.h"
#include "mylib.h"
#include "utente.h"
#include "sistemag.h"

void umain(int h)
{
	jmp_buf jmp;
	int i;
	char buf[200];
	sprintf(buf," pmsetjmp: testing setjmp/longjmp");
	videocpy(0,0,buf);	
	sprintf(buf,"|");
	videocpy(0,1,buf);	
	
	if((i = setjmp(jmp)) != 0) {
		sprintf(buf,"setjmp taken, with value %d",i);
		videocpy(0,12,buf);
		return;
	}
	sprintf(buf,"setjmp skipped,ready for longjmp");	
	videocpy(0,11,buf);	
	longjmp(jmp,22);
	// neve here
	while(1);
}


extern "C"
void cmain()
{
	bool risu;
	int proc,procHi;

	kernel_init();
	log_message("Welcome into Message Logging for OSSYS");

	ki_activate_p(umain,100,0,proc, risu);
	
	if(risu == false) 
		log_message("Non posso Allocare un Processo");

	kernel_go();
	
}

