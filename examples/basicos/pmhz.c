#include "mypc.h"
#include "mylib.h"
#include "8259.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// Calcolo della Frequenza di CPU: utilizziamo il timer
// senza interruzioni e con polling per stabilire il numero di cicli
// effettuati e quindi la frequenza della cpu
void cmain()
{	
	int32 tusec,tsech,tsecl;
	char buffer[200];
	video_clear();
	videocpy(1,0,"PMHZ calculates the cpu speed:");	

	tusec = get_cpu_clock();	
	tsech = tusec / 1000000;
	tsecl = tusec % 1000000;
	sprintf(buffer,"%4d.%3d MHz", tsech);
	videocpy(0,2,buffer);
}
