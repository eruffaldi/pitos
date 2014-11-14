#include "mypc.h"
#include "mylib.h"
#include <stdio.h>

void testkb();
void idtDump();
extern void testInt();

void fx()
{
	videocpy(50,0, "int 10 called ");
}

void cmain()
{
	sti();
	videocpy(1,0,"cmain() shows you the IDT and test DIV0");	
	idtSetEntry(10, fx);
	testInt();
	idtDump();
	testkb();
	halt();
}

void testkb()
{
	const char * sz1 = "Hello from DJGPP in PM, press any key or Escape to exit";	
	
	videocpy(0,23,sz1);
	while(1) {
		if(inportb(KB_STR) & 1) {
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