// Esempio PMC4.CPP 
// Testing:
//		Semaforo a 2 vie, Sincronizzazione tramite semaforo
//		Delay
//		Schedulazione e Rischedulazione con abbassamento della priorita' 
//
// Problemi: ancora con la memoria, la delete non funziona
//
#include "mypc.h"
#include <stdio.h>
#include <string.h>
#include "mylib.h"
#include "8259.h"
#include "utente.h"
#include "sistema.h"

int sincr, nsinc;

void process1(int h)
{
	char buf[200];
	int low, hi;
	get_tick(low,hi);
	sprintf(buf, "Process 1: Waiting 4 seconds tick:%u %u", hi,low);
	videocpy(0,1,buf);
	delay(4000);
	get_tick(low,hi);
	sprintf(buf, "Process 1 After Delay tick:%u %u, ready to signal Process 2", hi,low);
	videocpy(0,2,buf);
	sem_signal(sincr);
}

void process2(int h)
{
	videocpy(0,h,"Process2 will start to work after Process1 signals");
	char a[2] ="a";
	
	sem_wait(sincr);
	videocpy(0,h,"Process2 no more waiting...                       ");
	for(int i = 0; i < 1000000; i++) {
		a[0]++;
		videocpy(79,h,a);
	}
	videocpy(0,h,"Process2 finished to work                         ");
}

void process3(int h)
{
	void * pid;
	int pri;
	get_pid(pid);
	get_basepri(pri);
	char buf[200];
	sprintf(buf,"Process %p: base priority %d, try get sem", pid,pri);
	videocpy(0,h,buf);
	sem_wait(nsinc);
	for(int i = 0; i < 300000; i++) {
		sprintf(buf,"%d", i);
		videocpy(70,h,buf);
	}	
	sem_signal(nsinc);
}

extern "C"
void cmain()
{

	bool risu;
	int proc;
	
	kernel_init();	
	ki_activate_p(process1,100,0,proc, risu);
	
	if(risu != false) {
		ki_sem_ini(0, sincr,risu);
		ki_sem_ini(2, nsinc,risu);

		ki_activate_p(process3,20,1,proc, risu);
		ki_activate_p(process2,21,0,proc, risu);
		ki_activate_p(process3,22,2,proc, risu);
		ki_activate_p(process3,19,3,proc, risu);
		ki_activate_p(process3,18,2,proc, risu);
		ki_activate_p(process3,17,2,proc, risu);

		kernel_go();
	
	}
	
}

