#include "mypc.h"
#include "mylib.h"
#include "8259.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



void testkb();
void idtDump();

int count = 0;
int bExit = 0;
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

void kbCallback()
{
	int code = inb(0x60);
	char buffer[256];
	itoa(code, buffer,16);
	videocpy(0,21,buffer);
	if(code == 0x81) bExit = 1;
	if(code == 0x86) timer_stop();
	eoi(master);
}


void initKb()
{
	outb(0x64, 1);
}

void cmain()
{	
	// arrive with interrupt disabled
	// initialize the 8259A-1,2
	// install irq handlers in testing mode
	// activate interrupts
	videocpy(1,0,"pmc2 shows you 1sec timer and stop it with (key 4) ");	
	idtSetEntry(0x20+0, timerCallback);
	idtSetEntry(0x20+1, kbCallback);
	i8259_init();	
//	initTimer(59504);	// each 50 ms
	timer_start(59504);
	initKb();			// init kb interrupt
	sti();	
	testkb();
	halt();
}

void testkb()
{
	const char * sz1 = "Hello from DJGPP in PM, press any key or Escape to exit";	
	
	videocpy(0,23,sz1);
	while(!bExit) {
		//  && inportb(KB_STR) & 1
		if(0) {
			char buf[200];
			unsigned char syscode = inportb(KB_RBR);
			int code = syscode & 0x7F;
			if(syscode == 0x81) break;	// escape up
			if(syscode == 0x85) stopTimer();	
			sprintf(buf, "Key: %2x %3d %8s", code , code,syscode & 0x80 ? "make" : "break");						
			videocpy(0,24,buf);
		}
	}
}

void idtDump()
{
	int i;
	for(i = 0; i < 20; i++) {
		char buf[200];
		selector_t sel;
		offset_t off;
		int access = idt[i].h & 0xFFFF;
		off = (idt[i].l & 0xFFFF) | (idt[i].h & 0xFFFF0000);
		sel = (idt[i].l >> 16);		
		sprintf(buf,"%2d = %04x:%08x (%04x) [%08x,%08x]", i, (uint32)sel, off,access,
			idt[i].l,idt[i].h);
		
		videocpy(0,1+i,buf);
	}	
}