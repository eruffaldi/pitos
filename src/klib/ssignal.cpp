#include "mylib.h"
	#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "sistema.h"

struct {
	const char * nome;
	const char * desc;
} signaltable[] = 
{{"",""},
{"SIGHUP"," hangup, generated when terminal disconnects "},
{"SIGINT"," interrupt, generated from terminal special char "},
{"SIGQUIT"," (*) quit, generated from terminal special char "},
{"SIGILL"," (*) illegal instruction (not reset when caught)"},
{"SIGTRAP"," (*) trace trap (not reset when caught) "},
{"SIGABRT"," (*) abort process "},
{"SIGEMT"," (*) EMT instruction "},
{"SIGFPE"," (*) floating point exception "},
{"SIGKILL"," kill (cannot be caught or ignored) "},
{"SIGBUS"," (*) bus error (specification exception) "},
{"SIGSEGV"," (*) segmentation violation "},
{"SIGSYS"," (*) bad argument to system call "},
{"SIGPIPE"," write on a pipe with no one to read it "},
{"SIGALRM"," alarm clock timeout "},
{"SIGTERM"," software termination signal "},
{"SIGURG"," (+) urgent contition on I/O channel "},
{"SIGSTOP"," (@) stop (cannot be caught or ignored) "},
{"SIGTSTP"," (@) interactive stop "},
{"SIGCONT"," (!) continue if stopped "},
{"SIGCHLD"," (+) sent to parent on child stop or exit "},
{"SIGTTIN"," (@) background read attempted from control terminal"},
{"SIGTTOU"," (@) background write attempted to control terminal "},
{"SIGPOLL"," I/O possible, or completed "},
{"SIGXCPU"," cpu time limit exceeded (see setrlimit()) "},
{"SIGXFSZ"," file size limit exceeded (see setrlimit()) "},
{"SIGVTALRM6"," virtual time alarm (see setitimer) "},
{"SIGPROF"," profiling time alarm (see setitimer) "},
{"SIGWINCH"," (+) window size changed "},
{"SIGINFO"," (+) information request "},
{"SIGUSR1"," user defined signal "},
{"SIGUSR2"," user defined signal "},

};
// raise
// After the return, the syscall handler will check the software interrupts...
int	ki_raise(int _sig)
{
	if(_sig < 0 || _sig >= SIGCOUNT)
		return -1;
	esecuzione->sig_mask |= 1 << _sig;
	return 0;
}

 void	(*ki_signal (int _sig, void (*_func)(int)))(int)
 {
	if(_sig < 0 || _sig >= SIGCOUNT )
		return 0;
	sigfunc old = esecuzione->sig_handlers[_sig].handler;
	esecuzione->sig_handlers[_sig].handler = _func;
	
	char buf[100];
	sprintf(buf,"Signal Set %d %s <- %x -> %x",_sig,signaltable[_sig].nome,old,_func);
	log_message(buf);

	
	return old;
}

int getlsb(unsigned k)
{
	int n = 0;
	// exist better fx
	while((k & 1) == 0) k >>=1,n++;
	return n;
}

// handle signals
// 1) get the first to be executed (LSB) 
// 2) fix the stack:
//		 * system stack should have ESP to the new ESP in user stack
//		 * grow user stack with the new address and parameters...
//
// the user fx should return to the system process signals
// functions:
//	 check mask if any:
//	 salva stato
//	 process 
//	 carica stato
//   else iret..
// the user fx:
//	 add 4 ,esp
//   mov eax, process signals
//   int syscall
//   ret 
// ...
extern "C" 			
void handlesignals()
{
	unsigned sig = getlsb(esecuzione->sig_mask);
	sigfunc fx; 
	char buf[100];
	sprintf(buf, "Process %x Signal Mask %x, Selected %d %s = %x", esecuzione,esecuzione->sig_mask,sig,signaltable[sig].nome,(int)esecuzione->sig_handlers[sig].handler);
	log_message(buf);
	
	// todo fx for lsb extraction 
	// BUG BUG BUG
	if(sig >= SIGCOUNT )
		return ;
		
	fx	= esecuzione->sig_handlers[sig].handler;	
	esecuzione->sig_handlers[sig].handler = SIG_DFL;
	esecuzione->sig_mask &= ~(1 << sig);


	buf[0] = 0;
	for(int i = 0; i < 16;  i++)
		sprintf(buf+strlen(buf), " %x", (int)esecuzione->sig_handlers[i].handler);
	log_message(buf);


	if(fx == SIG_IGN) {
		// nothing appens...
		sprintf(buf, "Signal - %d - Ignored", sig);
		log_message(buf);		
		return;
	}
	else if(fx == SIG_DFL) {
		// default processing...
		sprintf(buf, "Signal - %d - Default", sig);
		log_message(buf);
		switch(sig)
		{
		case SIGQUIT:
			ki_terminate_p();
			break;
		}
		return;
	}
	else if(fx == SIG_ERR) {
		// explode
		char buf[100];
		sprintf(buf, "Signal - %d - Error", sig);
		log_message(buf);
		return;
	}

	add_apc(esecuzione, fx,(void*)sig);
}

extern "C" 
void ki___signalrespin()
{	
	// sistema lo stack utente
	// 1		id = 13
	// 2		sig index
	// 3		return 
	PSTACK sysesp = (PSTACK)esecuzione->context[_ESP];
	PSTACK useresp = (PSTACK)sysesp[3];
	useresp[1] = useresp[0];
	useresp += 1;
	sysesp[3] = (uint32)useresp;
	return;
}