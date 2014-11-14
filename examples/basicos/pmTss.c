// test TSS setting up
// test GDT INFORMATION...

int bExit = 0;
char * sysdesType[16] =
	{
	"res",
	"TSS16","LDT","TSS16 B", "CG-16", "TG",
	"IG-16", "TrG-16","res", "TSS32", "res", "TSS32 B","CG-32", "res", "IG-32", "TrG-32"
	};
	
char*	segdesType[16] =
	{
		"Data", "Data A", "Data RW", "Data RWA",		"DataEdn R", "DataEdn RA", "DataEdn RW", "DataEdn RWA",
		"Code", "Code A", "Code R", "Code RA", "Code C","Code CA", "Code CR", "Code CRA"
	};

int  page = 0;

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



void testkb();
void idtDump();

// initTimer inizializza il contatore 0 con
// la costante di tempo specificata,
// possiamo andare da 890ns a 58ms
void initTimer(int length1)
{
	// Tc = 840ns 
	// T = N * Tc
	// 0 0 - 1 1 - 0 1 1 0
	// id  - 1 1 - mode - 0 
	outb(0x43, 0x36);
	outb(0x40, length1 >> 8);
	outb(0x40, length1 & 0xFF);
}
void dumpGDTdes(int idx)
{
	char buf[200];
	gdtdes * dt = &gdt[idx];
	char * tipo;
	int type = dt->access ;
	if(!(type & 0x10)) 
		tipo = sysdesType[type & 0xF];
	else
		tipo = segdesType[type & 0xF];
	
	sprintf(buf,"%2d B=%08x L=%06x %c DPL=%d G=%d Type=%8s",
		idx, (dt->baselo| (dt->basemid<<16)|(dt->basehi<<24)),
		dt->limitlo|((dt->limithi & 0xF)<<16),
		dt->access & 0x80 ? 'P' : ' ',
		(dt->access & 0x60)>>5 , dt->limithi & 0x80? 1 : 0,tipo);
	videocpy(0,idx+4,buf);
}

void dumpIDTdes(int idx)
{
	char buf[200];
	idtdes * dt = (idtdes*)&idt[idx];
	char * tipo;
	int type = dt->access ;
	if(!(type & 0x10)) 
		tipo = sysdesType[type & 0xF];
	else
		tipo = segdesType[type & 0xF];
	
	sprintf(buf,"%2d %04x:%08x %c DPL=%d Type=%8s",
		idx, 
		dt->selector,
		(dt->offsetlo| (dt->offsethi<<16)),
		dt->access & 0x80 ? 'P' : ' ',
		(dt->access & 0x60)>>5 , 
		tipo);
	videocpy(0,idx,buf);
}


void kbCallback()
{
	int code = inb(0x60);
	char buffer[256];
	itoa(code, buffer,16);
	videocpy(0,21,buffer);
	eoi(master);
	if(code == 0x81) 
		bExit = 1;
	else if(code & 0x80)
	{
			// do the switch
			page = page == 0 ? 1 : 0;
			video_setpage(page);
	}
}


		


void initKb()
{
	outb(0x64, 1);
}

void cmain()
{	
	int i;
	// arrive with interrupt disabled
	// initialize the 8259A-1,2
	// install irq handlers in testing mode
	// activate interrupts
	videocpy(1,0,"pmc2 shows you 1sec timer and stop it with (key 4) ");	
	idtSetEntry(0x20+0, timerCallback);
	idtSetEntry(0x20+1, kbCallback);
	initTimer(59504);	// each 50 ms
	initKb();			// init kb interrupt
	i8259_init();	
	sti();	
	// start to dump the GDT
	video_setpage(0);
	for( i = 1; i < 7;i++) {
		dumpGDTdes(i);
	}
	
	video_setpage(1);
	// start to dump the GDT
	for( i = 0; i < 15;i++) {
		dumpIDTdes(i);
	}	
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

