#include "mypc.h"
#include <stdio.h>
#include <string.h>
#include "mylib.h"
#include "8259.h"
#include "utente.h"
#include "sistema.h"

int sincr;


void sysprocess(int h)
{
	h = 0;
	sti();
	log_message("sysproc: i'll wait the other process,wait key");
	log_show();	
	log_message("sysproc: Key Pressed,now wait over semaphore");
	sem_wait(sincr);
	log_message("sysproc: Sinchronized");
	
}

void umain(int h)
{
	char buf[200];
	sprintf(buf,"umain in CPL=3, h=%d, press key (not ESC) to give time to sysprocess", h);
	videocpy(0,24,buf);
	sysGetOsName(buf,sizeof(buf));
	videocpy(0,23,buf);
	sem_signal(sincr);
	videocpy(0,22,"after signal");
}


extern "C"
void cmain()
{

	kernel_init();
	log_message("Welcome into Message Logging for OSSYS");
	while(kb_waitkey() != 1);
	// Attenzione occorre creare almeno un processo che vada
	// a livello utente dato che al momento siamo a CPL 0
	// quindi occorre prendere usare una funzione di sistema allo stesso
	// livello di privilegio che inizializzi il tutto
	// basta chiamarla una sola volta.
	// Occorre inizializzare un task vuoto 
	// e passare ad esso con la carica_stato
	bool risu;
	int proc,procHi;
	ki_activate_p(umain,100,0,proc, risu);
	
	if(risu == false) {
		log_message("Non posso Allocare un Processo");
	}
	else {
		schedulatore();
		if(esecuzione == 0) 
			log_message("Nessun Task Schedulato");
		else {
			char	 buf[200];
			sprintf(buf, "Allocato processo=%08x idx=%d %08x", (int)esecuzione,proc
				, esecuzione->context[_ESP]);
			log_message(buf);
		}
	}

	ki_activate_p_level(sysprocess,0,20,0,procHi, risu);
	ki_sem_ini(	0, sincr,risu);
	char buf[200];
	sprintf(buf, "Semaforo Allocato %d", sincr);
	log_message(buf);
	kernel_go();
	
}

