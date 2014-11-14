#include <mylib.h>
#include <signal.h>
#include <sistema.h>
#include "syscall.h"
#include "8259.h"
#include "util.h"

#include "except.h"
#include "schedule.h"
#include <stdio.h>


extern "C" void handlesignals();
extern "C" void __signalrespin(void);



// Private Useful Functions
PSTACK alloca_stack(int livello, int size );
void dealloca_stack(PSTACK);
void dummyproc(int h);
void inserimento_coda_timer(richiesta* p);
extern "C" void acdriver_t();

// External Terminate for 
extern "C" void terminate_p();

// Variabili Statiche
static char sysStack[4096];

// Variabili Globali
int proc_count;
des_proc desproc[MAXPROCS];
des_sem  dessem[MAXSEMS];
des_proc * esecuzione = 0;
des_proc * pronti = 0;
richiesta * des_timer = 0;


// Funzioni viste a livello sistema
void kernel_init()
{
	// initialize logging...
	i8259_init();
	log_init();
	sti();
	exceptions_install();	
			
	// initialize processes
	for(unsigned i = 0; i < sizeof(desproc)/sizeof(desproc[0]); i++)
		desproc[i].stato = FREE;

	for(unsigned j = 0; j < MAXSEMS; j++)
		dessem[j].busy = false;
		
	// initialize TSS to any stack
	_TSS.ESP0 = (int)sysStack+4095;
	_TSS.SS0  = 0x10;		// data system level
	
	// Inizializza la SYSCALL
	idtSetEntryAsm(SYSCALLTIPO, &syscalle,3);

	timer_stop();

	idtSetEntryAsm(0x20, &acdriver_t,0);
	
	HPROCESS dummy;
	bool risu;
	ki_activate_p_level(dummyproc,0,3000,0,dummy, risu);

	
}

void kernel_go()
{
	if(!esecuzione)
		schedulatore();
	
	if(!esecuzione) {
		log_message("Non ci sono processi disponibili all'esecuzione. HALT");
		log_show();
	}
	else {
		timer_start(59504);		// 50ms
		carica_stato();
	}
}



void give_num(int & n)
{
	n = proc_count;
}

// *******************************************************************************************************
// IMPLEMENTAZIONI SYSCALL
extern "C"  {

void ki_sysGetOsName(char * p, int len)
{
	char buf[200];
	sprintf(buf, "sysGetOsName p=%08x len=%d", (int)p, len);
	log_message(buf);
	if(!len || !p) return;
	char * s = "MOPPE OPS SYS";
	while(--len)
		*p++ = *s++;
	*p = 0;		
}


void ki_activate_p(void (*f)(PROCESSPARAM h), PROCESSPARAM h, int prec, HPROCESS & idx, bool & risu)
{	ki_activate_p_level(f,h,prec,3,idx,risu); }

void ki_activate_p_level(void (*f)(PROCESSPARAM h), PROCESSPARAM h, int prec, int level,HPROCESS & idx, bool & risu)
{
	clamp(level,0,3);
	if(proc_count == MAXPROCS) {
		risu = false;
		return;
	}
	proc_count++;
	
	// cerca processo libero...
	des_proc * p = 0;
	for(int i = 0; i < MAXPROCS; i++)
		if(desproc[i].stato == FREE) {
			p = &desproc[i]; 
			idx = i;
			break;
		}
		
	// inizializza il processo allocando lo stack
	for(int j = 0; j < 3; j++)
		p->stk[j] = alloca_stack(j,4096);	
		
	PSTACK userstack = p->stk[2];
		
	// check stack allocation size	
	p->next = 0;
	p->stato = PRONTO;
	PSTACK esp = p->stk[0];
	
	// riempiamo il system stack in modo da fare
	// una iret alla funzione f
	// EIP = f , CS = usercode
	// inoltre, la f deve avere su stack il parametro...
	if(level == 3) {
	userstack -= 2;
	userstack[0] = (int)&terminate_p;		// problemino!!!
	userstack[1]   = h;					// attenzione anche alla dimensione di h (32/64bit)
	
	esp -= 5;
	esp[0] = (int)f;
	esp[1] = (int)USERCODESEL;
	esp[2] = 1 << 9; // DEFAULT EFLAGS
	esp[3] = (int)userstack;
	esp[4] = (int)USERDSSEL;
	
	}
	else {
		esp -= 5;
		esp[0] = (int)f;
		esp[1] = (int)SYSTEMCODESEL;
		esp[2] = 1 << 9; // IF
		esp[3] = (int)&ki_terminate_p;
		esp[4] = (int)h;
	}
	
	// now can fill in the registers
	p->context[_ESP] = (int)esp;
	p->context[_DS]  = USERDSSEL;
	p->context[_ES]  = USERDSSEL;
	p->context[_GS]  = USERDSSEL;
	p->context[_FS]  = USERDSSEL;
	p->context[0]  = 0x100;
	p->prec = p->baseprec = prec;
	p->tick = 0;
	p->tickTotal = 0;
	{
	char buf[200];
	sprintf(buf, "activate_p: %d stacks: %p %p prec: %d fx: %p",
		idx, p->stk[0],p->stk[2], prec, f);
	log_message(buf);
	}
	p->sig_mask = 0;
	for(int i = 0; i < SIGCOUNT; i++)
		p->sig_handlers[i].handler = SIG_DFL;
	inserimento_coda(pronti, p);
	risu = true;
	
}

// ki_terminate_p
//
// Routine Di Terminazione Del Processo Corrente
// Libera la memoria del Processo ed invoca la schedulazione
// di un nuovo processo
// Se non ci sono processi pronti allora abbiamo molta crisi!!!
void ki_terminate_p()
{
	des_proc * p = esecuzione;

	// dealloca gli stack e la tabella
	p->stato = FREE;
	
	
	for(int i = 0; i < 3; i++)
	;//	dealloca_stack(	p->stk[i]);
	 	
	proc_count --;
	
	// if no more that's impossible
	if(pronti == 0) {
		log_message("KERNEL: no more processes to run, halted");
		sti();
		log_show();
		timer_stop();
		while(1);
	}
	else {
		char buf[60];
		sprintf(buf, "Process %p terminated, %d remaining",esecuzione, proc_count);
		log_message(buf);		
	}		
	schedulatore();
}


void ki_sem_wait(HSEM hsem)
{
	if(!inRange(hsem,0,MAXSEMS-1)) return;
	des_sem * psem = &dessem[hsem];
	
	psem->count--;
	if(psem->count < 0) {
		inserimento_coda(psem->coda, esecuzione);
		schedulatore();
	}
}

void ki_sem_signal(HSEM hsem)
{
	if(!inRange(hsem,0,MAXSEMS-1)) return;
	des_sem * psem = &dessem[hsem];
	des_proc * lavoro;
	
	psem->count++;
	if(psem->count <= 0) {
		rimozione_coda(psem->coda, lavoro);
		if(lavoro->prec <= esecuzione->prec) {
			inserimento_coda(pronti, esecuzione);
			esecuzione = lavoro;
			esecuzione->stato = PRONTO;
		}
		else
			inserimento_coda(pronti,lavoro);
	}
}

void ki_sem_ini(int valore, HSEM & idx,bool & risu)
{
	des_sem * p = 0;
	for(int i = 0; i < MAXSEMS; i++)
	{
		if(!dessem[i].busy)  {
			idx = i;
			p = &dessem[i];
		}
	}
	if(!p) {
		risu = false;
		return;
	}
	p->count = valore;
	p->busy = true;
	p->coda = 0;
}

void ki_get_tick(int & low, int & hi)
{
	rdtsc(low,hi);
}
 void ki_get_pri(int & pri)
{

	pri = esecuzione->prec;
}
 void ki_get_basepri(int & pri)
{

	pri = esecuzione->baseprec;
}

void ki_get_pid(void * & p)
{

	p = esecuzione;
}


// attesa in millisecondi
void ki_delay(int n)
{
	// implementare usando i timer...
	richiesta * p = new richiesta;
	p->dattesa = n/50;
	p->proc = esecuzione;
	inserimento_coda_timer(p);
	schedulatore();		
}



} // extern "C"

void inserimento_coda_timer(richiesta* p)
{
richiesta * r, *prec;

	r = des_timer;
	prec = 0;
	
	while(r) {
		if(p->dattesa > r->dattesa) {
			p->dattesa -= r->dattesa;
			prec = r;
			r = r->pric;
		}
		else
			break;			
	}
	
	// sistema puntatore al successivo
	p->pric = r;
	p->proc->next = 0;
	
	// sistema il puntatore del precedente
	if(!prec) 
		des_timer = p;
	else
		prec->pric = p;
		
	// sistema il time to go del successivo	
	if(r) 
		r->dattesa -= p->dattesa;
}



// al trigger del timer...
extern "C"
void cdriver_t()
{
	static int rescheduler = 0;
	richiesta *p;

	if(des_timer)
		des_timer->dattesa--;
	
	while(des_timer && des_timer->dattesa == 0) {
		p = des_timer;
		des_timer = p->pric;
		
		// inserisci il processo nella coda dei pronti
		p->proc->stato = PRONTO;
		inserimento_coda(pronti, p->proc);
		// ripulisci memoria
		 delete p;
	}

	if(!esecuzione)
		log_message("PANIC ESECUZIONE 0");
	esecuzione->tick++;
	
	if(esecuzione->sig_mask) 
		handlesignals();
		
	if(++rescheduler > RESCHEDULER) {
		// update the scheduling adjusting the priorities of all processed			
		rescheduler = 0;
		esecuzione->tickTotal += esecuzione->tick;
		inserimento_coda(pronti,esecuzione);
		reschedule();
		schedulatore();
	}
	else if(esecuzione->tick > QUANTUM) {
		inserimento_coda(pronti,esecuzione);
		schedulatore();
	}
	eoi(master);
}


// *********************************************************************************************************
// DEBUGGING
void dumpSysCalls(int first,int last)
{
	// Stampa SysCallTable
	if(first < 0) first = 0;
	if(last < first) last = first;

	for(int i = first; i <= last;i++)
	{
		char buf[200];
		sprintf(buf, "syscall %d = %08x", i, syscall_table[i]);
		log_message(buf);
	}
}

// dump context
enum ContextDump { STACK = 1, DUMPESP=2, GENERAL=4 };
void dumpContext(des_proc * p, int flags)
{
	char buf[100];

	if(flags & GENERAL) 
		for(int i = 0; i < NUMREG;i++)
		{
			sprintf(buf, "%d = %08x", i,p->context[i]);
			log_message(buf);
		}
		
	// then the second part, a small stack
	if(flags & DUMPESP) {
		sprintf(buf, "ESP = %08x", p->context[_ESP]);
		log_message(buf);
	}
	
	if(flags & STACK) {
		int * esp = (int*)(p->context[_ESP]);
		for(int j = 0; j < 8; j++)
		{
			sprintf(buf, "+%d %08x", j,esp[j]);
			log_message(buf);		
		}
	}
	/*
	// check the user stack
	if((esp[1] & 3) != 0) {
		sprintf(buf, "user stack=%04x:%08x", esp[4],esp[3]);
		log_message(buf);
		int * espu = (int*)esp[3];
		for(int k = 0; k < 3; k++)
		{
 			sprintf(buf, "+%d %08x", k, espu[k]);
			log_message(buf);
		}
	}
	*/
}

// Private functions
void dummyproc(int h)
{
	h = 0;
	int lavoro;
	sti();
	do {		
		give_num(lavoro);
	} while(lavoro > 1);	
}

PSTACK alloca_stack(int livello, int size )
{
	videocpy(0,10, "Allocazione STACK");
	return (PSTACK)(new char[size+512]+size);
}

void dealloca_stack(PSTACK aa)
{
	delete aa;
}

// inserisci_coda
// inserisce dentro una coda tenendo conto della
// priorita', il processo p si trovera' alla fine
// della lista dei processi di pari priorita'
void inserimento_coda(des_proc * &coda, des_proc * p)
{
	if(!p) return;

	des_proc * r, * prec;
	prec = 0;
	r = coda;
	
	while(r) {
		if(p->prec < r->prec) {
			// inseriscilo prima di r
			break;
		}
		else
		{
			prec = r;
			r = r->next;
		}
	}
	p->next = r;
	if(prec)
		prec->next = p;
	else
		coda = p;	
}


// inserisci_coda_testa
// inserisce dentro una coda tenendo conto della
// priorita', il processo p si trovera' alla fine
// della lista dei processi di pari priorita'
// Va in testa a quelli di pari priorita'
void inserimento_primo_coda(des_proc * &coda, des_proc * p)
{
	if(!p) return;

	des_proc * r, * prec;
	prec = 0;
	r = coda;
	
	while(r) {
		if(p->prec <= r->prec) {
			// inseriscilo prima di r
			break;
		}
		else
		{
			prec = r;
			r = r->next;
		}
	}
	p->next = r;
	if(prec)
		prec->next = p;
	else
		coda = p;	
}

void rimozione_coda(des_proc *& coda, des_proc * &p)
{
	p = coda;
	if(!coda) {return;}
	coda = coda->next;
	p->next = 0;	// DEBUG
}

//******************************************************************************************************
// SYSTEM CALL TRACING...
extern "C" {

int count = 0;

// traceSyscall
// ESP points to ESP (push esp)
// pastack is the user parameter stack, first item is the CALLID
void traceSyscall(int* esp,int*pastack)
{
	return;
	char buf[100];
	count++;
	int n = pastack[0];
	int params = syscall_params[n];
	int CPL = esp[2]&3;
	sprintf(buf, "syscall: #%d CPL=%d target=%08x params=%d (%d called)",  n,CPL,syscall_table[n],
		params,count);
	log_message(buf);
/*	
	// Print The User Stack
	int i;
	for( i = 0; i < params; i++) {
		sprintf(buf, "p(%d) = %08x", i,pastack[i]);
		log_message(buf);
	}


	// Print The System Stack 
	for(i = 0; i < 7; i++) {
		sprintf(buf, "ps(%d) = %08x", i,esp[i+1	]);
		log_message(buf);
	}
	// Print TSS.ESP0 & TSS.ESP
	sprintf(buf, "TSS ESP0=%08x ESP=%08x", _TSS.ESP0, _TSS.ESP);
	log_message(buf);
	
	// dump a context
	dumpContext(esecuzione,STACK|GENERAL|DUMPESP);

*/

}

void badSysCall(int n)
{
	char buf[200];
	sprintf(buf, "Bad System Call %d", n);
	log_message(buf);
	log_show();
	sti();
	while(1);
}

}	// extern "C"

// aggiunge una chiamata a procedura remota al programma
void add_apc(des_proc*p, void * fx, void * param)
{
	PSTACK useresp,sysesp;
	// Nello Stack di sistema dobbiamo posizionare l'indirizzo della nuova funzione di rientro
	// ovvero fx
	// Final User Stack:
	//		__signalrespin
	//		sig
	//		current return address (from system stack)
	// Final System Stack
	//		fx
	//		CS
	//		EFLAGS
	//		esp (fixed)
	//		SS
	sysesp  = (PSTACK)p->context[_ESP];
	useresp = (PSTACK)sysesp[3];
	
	useresp -= 3;
	useresp[0] = (uint32) & __signalrespin;	
	useresp[1] = (uint32) param;			
	useresp[2] = sysesp[0];				
	sysesp[0]  = (uint32) fx;
	sysesp[3]  = (uint32) useresp;

}


int outPage = 0;
int lastRow = 1;
int lastCol = 0;
void ki_print(char * buf, int page)
{
	if(page == 2) {
		log_message(buf);
		return;
	}
	int op = video_setpage(page);
	char c;
//	log_message("ki_print");
	// check user buffer!!!!
	while(*buf != 0) {
		c = *buf++;
		switch(c) {
		case '\n':
			lastRow++, lastCol = 0;			
			break;
		case '\r':
			lastCol = 0;		
			break;
		case 8: // BKSP
			if(lastCol != 0) lastCol--;	
			else if(lastRow != 0) lastRow --;
			break;
		case '\t':
			lastCol += 8;
			if(lastCol >= 80)
				lastCol -= 80;
			break;
		default:
			if(lastCol >= 80)
				lastRow++,lastCol = 0;
			if(lastRow >= VIDEOROWS)	
				lastRow = 0;		
			video_char(lastCol,lastRow,c);
			lastCol++;
		}

	}
	video_setpage(op);
}

