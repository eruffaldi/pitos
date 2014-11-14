// Lato Sistema
#ifndef SYSTEM_H
#define SYSTEM_H

#define HAS_SIGNAL 

#ifdef HAS_SIGNAL
#include <signal.h>
#include "ssignal.h"
#endif

typedef long * PSTACK;
enum StatoProcesso { PRONTO, BLOCCATO,RUNNING,FREE};

#define NUMREG 12
#define _ESP 	28/4
#define _DS		44/4
#define _ES		32/4
#define _FS		36/4
#define _GS		40/4

#define USERDSSEL		0x20|3
#define USERCODESEL		0x18|3
#define SYSTEMCODESEL	0x8

#include "sistemag.h"


#define QUANTUM		10
#define RESCHEDULER		50		
// 50ms * QUANTUM = ...
struct des_proc
{
	int context[NUMREG];
	PSTACK stk[3];		// tre stack
#ifdef HAS_SIGNAL	
	signal_mask_t sig_mask;
#endif 

	int prec;
	int baseprec;
	des_proc * next;
	StatoProcesso stato;
	int tick;			// current execution
	int tickTotal;		// and upon last RESCHEDULE
	// Stack Base Addresses for Deallocation
	struct signal_t sig_handlers[SIGCOUNT];
#ifdef HAS_VM
	int tabella;
#endif
} ;

typedef int HSEM;

struct des_sem
{
	des_proc * coda;
	int count;
	bool busy;			// potremmo usare una bitmask...
};

struct richiesta {
	richiesta * pric;
	des_proc  * proc;
	int dattesa;
};

extern myTSS _TSS;

extern "C" {
extern void ki_activate_p(void (*f)(PROCESSPARAM h), PROCESSPARAM h, int prec, HPROCESS & idx, bool & risu);
extern void ki_activate_p_level(void (*f)(PROCESSPARAM h), PROCESSPARAM h, int prec, int level,HPROCESS & idx, bool & risu);
extern void ki_enqueue_init(HPROCESS &idx);
extern void ki_terminate_p();
extern void ki_get_pid(void * & p);
extern void ki_get_pri(int & pri);
extern void ki_get_basepri(int & pri);

extern void ki_sem_ini(int valore, HSEM & idx,bool & risu);
extern void ki_sem_signal(HSEM idx);
extern void ki_sem_wait(HSEM idx);
extern void ki_sysGetOsName(char *  p, int len);
extern void ki_get_tick(int & low, int & hi);
extern void ki_delay(int ms);
extern void ki_print(char * buf, int page);
}

void inserimento_coda(des_proc * & coda, des_proc * proc);
void rimozione_coda(des_proc * & coda, des_proc * & out);
void inserimento_primo_coda(des_proc * & coda, des_proc * proc);
void give_num(int & n); 
void add_apc(des_proc*, void * fx, void * param);

#define MAXPROCS 	20
#define MAXSEMS 	20

extern des_proc desproc[MAXPROCS];
extern des_sem  dessem[MAXSEMS];
extern int      proc_count;
extern des_proc * pronti;
extern des_proc * esecuzione;


#endif 