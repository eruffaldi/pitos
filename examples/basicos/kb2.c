#include "mypc.h"
#include "mylib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// BUG BUG BUG: fmod assente, sprintf non ammette X
static int start = 0xB8000;


void videocpy2(int c, int r, const char *sz)
{
	short * p = (short*)(start+160*r+2*c);
	while(*sz != 0)
	{
		if(*sz == '\n') break;
		*p++ = (*sz++ & 0xFF)| 0x700;
	}	
}
void amain()
{
	const char * sz1 = "Hello from DJGPP in PM, press any key or Escape to exit";	
	const char * sz2 = "CPU Halted";	
	int i;
	
	videocpy2(0,1,sz1);
	while(1) {
		if(inportb(KB_STR) & 1) {
			char buf[200];
			unsigned char syscode = inportb(KB_RBR);
			int code = syscode & 0x7F;
			if(syscode == 0x81) break;	// escape up
			
			/* be careful not fully complieant */
			sprintf(buf, "Key: %2x %3d %8s", code , code,syscode & 0x80 ? "make" : "break");			
			
			/*
			itoa(code, buf, 16);
			strcat(buf, " ");
			strcat(buf, syscode & 0x80 ? "make ": "break");
			*/
			videocpy(0,2,buf);
		}
	}
	videocpy(0,3,sz2);
	halt();
}
