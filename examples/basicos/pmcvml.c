#include "mypc.h"
#include <stdio.h>


char * start = (char*)0x000b8000;

void videocpy(int c, int r, const char *sz)
{
	short * p = (short*)(start+160*r+2*c);
	while(*sz != 0)
	{
		*p++ = (*sz++ & 0xFF)| 0x700;
	}
}

double fmod(double a,double b)
{
	return 0;
}

void cmain()
{
	int *p;
	int i,j;
	char buf[100];
	sti();
p = (int)0;

	videocpy(1,0,"Showing the VM Table");
	videocpy(1,1,"Directory     FirstPage      SecondPage");	
	for(i = 0; i < 22; i++)
		{
		sprintf(buf, "%08x%c",p[i] & 0xFFFFF000,p[i] & (1<<5) ? 'A':' ');
		videocpy(1,i+2,buf);
		}
	for(  j = 0; j < 4; j++) {
		p = (int)0x1000*(1+j);
		for(i = 0; i < 22; i++)
		{
			sprintf(buf, "%08x%c",p[i] & 0xFFFFF000,p[i] & (1<<5) ? 'A':' ');
			videocpy((j+1)*15,i+2,buf);
		}
	}
	halt();
	while(1);
}


char _TSS[200];