// PMSIG1
// Dimostrazione delle funzionalita' dei signal come chiamate a procedura asincrone.
// Il processo utente identificato dalla funzione umain gestisce i due segnali SIGQUIT e SIGTERM
// e solleva subito SIGTERM.
// Il segnale SIGQUIT puo' essere generato dal tasto 5, per default chiama terminate_p, quindi 
// dato che viene gestito e non riattivato occorre premere due volte 5 per uscire.
//
#include "mypc.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "mylib.h"
#include "8259.h"
#include "utente.h"
#include "sistema.h"
#include "schedule.h"


void mysig(int h)
{
	char buf[200];
	sprintf(buf, "Signal Handler %d, Bye Bye", h);
	videocpy(0,20 ,buf);
}

void umain(int h)
{
	char buf[200];
	sprintf(buf,"umain in CPL=3, installing signal %d = %x",(int)SIGQUIT,(int)&mysig);
	videocpy(0,21,buf);	
	signal(SIGQUIT, &mysig);
	signal(SIGTERM, &mysig);
	raise(SIGTERM)	;
	sprintf(buf,"after raise");
	videocpy(0,19,buf);	
	int k =10;
	int q = 0;
	k /= q;
	while(1);
}


extern "C"
void cmain()
{
	bool risu;
	int proc,procHi;

	kernel_init();
	log_message("Welcome into KVM on PITOS");

	ki_activate_p(umain,100,0,proc, risu);
	
	if(risu == false) 
		log_message("Non posso Allocare un Processo");
	kernel_go();
	
}

