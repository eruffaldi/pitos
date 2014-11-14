#include "mypc.h"
#include "mylib.h"
#include "8259.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int count = 0;
void timerCallback()
{
	count++;
	if (count == 1000/50) 
	{
	unsigned long now ;
	char buffer[200];
	
	count = 0;
	rdtscl(now);
	itoa(now, buffer,10);
	videocpy(0,22,buffer);	
	strcat(buffer, "    ");
	}
	eoi(master);
}


void cmain()
{	
	int old;
	
	videocpy(1,0,"PMCLOG shows you logging using the multipage video mode");	
	
	old = video_setpage(1);
	videocpy(1,0,"This is page 2, press a key");
	kb_waitkey();
	video_setpage(old);
	videocpy(1,2,"This is back to page 1, press ESCAPE to see Logging");
	
	// inizializzazione...
	i8259_init();	
	log_init();

	idtSetEntry(0x20+0, timerCallback);
	timer_start(59504);	// each 50 ms	
	
	sti();	
	log_message("Hello World");
	log_message("Logging is Working...");
	
	// attesa
	while(1) ;
}

