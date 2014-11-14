#include "mylib.h"
#include <stdlib.h>
#include <stdio.h>
#include "sistema.h"


// schedulatore
// Analizza la coda dei pronti e pone in esecuzione il 
// processo a piu' altra priorita', sfortunatamente
// occorre sistemare per abbassare la priorita' di
// processi troppo usati usando un meccanismo di punizione
// Unix:
// priorita' > 0 user
// priorita' < 0 kernel
// Timing:
// ad ogni clock aumento 
void schedulatore()
{	
	static int called = 0,switched = 0;
	called++;
	if(pronti != 0) {
		des_proc * old = esecuzione;
		if(esecuzione != 0)
			esecuzione->tickTotal += esecuzione->tick;
		esecuzione = pronti;
		esecuzione->stato = RUNNING;

		int tick = esecuzione->tick;
		esecuzione->tick = 0;
		pronti = pronti->next;
		esecuzione->next = 0;
		switched++;

		char buf[200];
		if(1 && esecuzione != old) {
		sprintf(buf,"Switch: %08x prec=%d/%d time=%d/%d", (int)esecuzione,esecuzione->prec,esecuzione->baseprec,	
			tick,esecuzione->tickTotal); 		
		log_message(buf);
		}
	}
	char buf[30];
	sprintf(buf, "%d/%d", switched,called);
	videocpy(70,24,buf);	
}

void reschedule()
{

//	log_show();
//	log_message("Rescheduling...");
	

	des_proc * p = pronti;
	
des_proc * pp[100];
	int n = 0;
	while(p != 0)
	{
		p->prec = p->baseprec + p->tickTotal/10;
		p->tickTotal = 0;
		pp[n++] = p;
		p->stato = PRONTO;
		p = p->next;
	}
	
	pronti = 0;
	for(int i = 0; i < n; i++)
		inserimento_coda(pronti, pp[i]);
//	log_message("Rescheduling ended");
}
