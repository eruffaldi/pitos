// kmain
// Example of KVM invocation from PITOS
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <mylib.h>
#include <8259.h>
#include <sistemag.h>
#include <utente.h>
#include <schedule.h>
#include "mypc.h"

extern "C" {
	int StartJVM(int argc, char* argv[]);
	int KVM_Start(int argc, char* argv[]);
	void KVM_Cleanup(void);
}

void umain(int )
{
	int result;
	int argc = 1; 	char * argv[] = {"test",0}; 

	print("Welcome from User Process just before Running JAVA",2);
	result = StartJVM(argc, argv);
	while(1);	
}


void fx()
{
}

extern "C"
void cmain()
{
	bool risu;
	int proc;	
	
	kernel_init();
	video_clear();
	log_message("Welcome into KVM on PitOS Release");
	log_show();
	
	ki_activate_p(umain,100,0,proc, risu);
	if(risu == false) 
		log_message("Non posso Allocare un Processo");
	kernel_go();		
}


