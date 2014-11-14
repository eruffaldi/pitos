// PMPRINT.CPP
// Testing: fprintf e putchar
// Testing: eccezioni a livello di processo utente e kill del processo
//          visualizzazione dell'offending opcode
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "mypc.h"
#include "mylib.h"
#include "utente.h"
#include "sistemag.h"

void umain(int h)
{
	
	char * a = new char[10];
	fprintf(stderr,"message to system logging");
	while(1);
	fprintf(stdout,"message to Application Viewport\nTwo Lines\nThree");
	putchar('\r');
	putchar('*');	
	
	// testing invalid opcode exception over cli
	if(h == 0)
		cli();
	else
		video_setpage(3);
	
	while(1);
}


extern "C"
void cmain()
{
	bool risu;
	int proc;

	kernel_init();
	log_message("Welcome into Message Logging for OSSYS");
	
	
	//ki_activate_p(umain,0,0,proc, risu);
	ki_activate_p(umain,100,0,proc, risu);
	
	if(risu == false) 
		log_message("Non posso Allocare un Processo");

	kernel_go();

}

