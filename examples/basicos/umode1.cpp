// UMODE1.CPP
// Testing: usermode
#include "mypc.h"
#include "mylib.h"
#include "utente.h"
#include "sistemag.h"

void umain(int h)
{
}


extern "C"
void cmain()
{
	bool risu;
	int proc;

	kernel_init();
	log_message("Welcome into Message Logging for OSSYS");
		
	ki_activate_p(umain,100,0,proc, risu);
	
	if(risu == false) 
		log_message("Non posso Allocare un Processo");

	kernel_go();

}

