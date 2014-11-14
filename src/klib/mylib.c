// somelibfx
#include <mylib.h>
#include <mypc.h>
#include <8259.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include "ssignal.h"
#include <stdlib.h>
#include <stdarg.h>

#include "systime.h"

int kb_has_callback = 0;
int kb_changed;
int kb_last;
uint32 clocksperusec10;
// got some bug with the VMWARE!!
// with the first and the last positions


// 25 = 16+9
#define TRAMPFIX 0x19+5+1
#define TRAMPFIXPARAM 0x19+1

unsigned char trampoline[] = {
	0x1E,0x6,0xF,0xA0,0xF,0xA8,		// push ds,push es, push fs,push gs
	0x60,		// pusha
	0x66,0xb8,0x10,0x00,		// mov ax, 0x10
	0x66,0x8e,0xd8,			    // set selectors
	0x66,0x8e,0xc0,
	0x66,0x8e,0xe0,
	0x66,0x8e,0xe8,
	0xfc,						// cld
	0x54, 						// push esp
	0x68,0x0,0x0,0x0,0x0,		// push INT INDEX
	0xb8,0x0,0x0,0x0,0x0,		// mov eax, FIXHERE
	0xff,0xd0,					// call eax
	0x58,0x58,					// pop eax, pop eax
	0x61,						// popa
	0x0f,0xa9,0xf,0xa1,0x7,0x1f, // pop sregs
	0xcf						// iret
	};

typedef struct
{
	char buf[sizeof(trampoline)];
} tIdtTrampoline;

tIdtTrampoline idtTramps[256];



double fmod(double a,double b)
{
	return 0;
}

char *
itoa(int value, char *string, int radix)
{
  char tmp[33];
  char *tp = tmp;
  int i;
  unsigned v;
  int sign;
  char *sp;

  if (radix > 36 || radix <= 1)
  {
//    errno = EDOM;
    return 0;
  }

  sign = (radix == 10 && value < 0);
  if (sign)
    v = -value;
  else
    v = (unsigned)value;
  while (v || tp == tmp)
  {
    i = v % radix;
    v = v / radix;
    if (i < 10)
      *tp++ = i+'0';
    else
      *tp++ = i + 'a' - 10;
  }

	if(string == 0) return  0;
  sp = string;

  if (sign)
    *sp++ = '-';
  while (tp > tmp)
    *sp++ = *--tp;
  *sp = 0;
  return string;
}


// level 0 entry...
void idtSetEntryAsm(int idx, void * fx,int level)
{
	int l = (int)fx;
	cli();
	idt[idx].l = 0x80000 | (l & 0xFFFF);
	idt[idx].h = (l & 0xFFFF0000) | 0x8E00 | (level << 13);		
	sti();
}

void idtSetEntry(int idx, void (*fx)(int))
{
	int i;uint32 l;
	if(idx < 0 || idx >= 256) return;
	
	l = (uint32)&idtTramps[idx].buf[0];
	
	cli();
	
	// copy the code and fix the pointer
	for(i = 0; i < sizeof(trampoline); i++) 	idtTramps[idx].buf[i] = trampoline[i];
	*((long*) (&idtTramps[idx].buf[TRAMPFIX])) = (long)fx;
	*((long*) (&idtTramps[idx].buf[TRAMPFIXPARAM])) = idx;

	
	// adjust idt to the trampoline buffer
	idt[idx].l = 0x80000 | (l & 0xFFFF);
	idt[idx].h = (l & 0xFFFF0000) | 0x8E00;
	sti();
		
}


static void rdtsc_delay(unsigned long loops)
{
	unsigned long bclock, now;
	
	rdtscl(bclock);
	do
	{
		rdtscl(now);
	}
	while((now-bclock) < loops);
}

// udelay
// delay in usecs so we use the value clock_per_usec10
void udelay(unsigned long usec)
{
	// CAN OVERFLOW 
	rdtsc_delay(1000000);
	// FIX FIX FIX(sec* clocksperusec10);
}


// Logging Protocol:
// print each line over the screen
int logRow = 0;
int logPage = 2;
int VIDEOROWS =	25;

void log_message(char * sz)
{
	int op = video_setpage(logPage);
	video_eraseline(logRow);
	videocpy(1,logRow,sz);
	if(++logRow >= VIDEOROWS) {
		logRow = 0;
	}	
	video_setpage(op);
}

void log_show()
{
	video_setpage(logPage);
}

void loggingKbCallback(int h);

void log_init()
{
	// init kb
	kb_setcallback(loggingKbCallback);
	video_setpage(0);
	logRow = 0;
}

void loggingKbCallback(int  h)
{
	int code = inb(KB_RBR);
	eoi(master);
	kb_changed = 1;
	kb_last = code;
	if(code == 0x1) {
		// swap the video page		
		if(video_getpage() != logPage) 
			video_setpage(logPage);
		else 
			video_setpage(0);	
	}	
	else if(code == 0x86) {
		ki_raise(SIGQUIT);		// al processo corrente!!!
	}
}

// initTimer inizializza il contatore 0 con
// la costante di tempo specificata,
// possiamo andare da 890ns a 58ms
void timer_start(short length1)
{
	// Tc = 840ns 
	// T = N * Tc
	// 0 0 - 1 1 - 0 1 1 0
	// id  - 1 1 - mode - 0 
	outb(0x43, 0x36);
	outb(0x40, length1 & 0xFF);
	outb(0x40, length1 >> 8);
	i8259_unmask(0);
}

void timer_stop()
{
	outb(0x43,0);
	i8259_mask(0);
	// outb(0x21,inb(0x21) | 1);	// mask	
}

// Keyboard

// wait the release
unsigned kb_waitkey()
{
	if(!kb_has_callback) {
	while(1) {
		int code;
		if((inb(KB_STR) & 1) && ((code = inb(KB_RBR)) & 0x80) == 0x80)
			return code & 0x7F;
	}
	}
	else {
		while(1) 
			if(kb_changed == 1) {
				kb_changed = 0;
				if((kb_last & 0x80) == 0x80) return kb_last & 0x7F;
			}
	}
}

void kb_empty()
{
	while(inb(KB_STR) & 2) ;
}

void kb_setcallback(void (*f)(int h))
{
	idtSetEntry(0x20+1, f);
	outb(KB_STR, 1);
	i8259_unmask(1);
	kb_has_callback = 1;
}
myTSS _TSS;

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
	videocpy(0,1,buf);
}



// get_cpu_clock()
//
// Returns:
//	 >= 0	number of operations for each second (AT MOST 4GHZ)
//   -1		not set
//	 -2     slow
//   -3     fast
#define TESTTICK 59660

#ifndef USE_CMOS_CLOCK_CALC
uint32 get_cpu_clock()
{
	uint32 sl,sh,el,eh;
	uint16 length = TESTTICK;
	int count = 0;
	
	// clean speaker and set freeze
	outb(0x61, (inb(0x61) & ~0x02)|0x01);
	
	// use timer 2 
	outb(0x43, 0xb0);				
	outb(0x42, length & 0xFF);
	outb(0x42, length >> 8);		
	
	rdtsc(sl,sh);
	do {
		count++;
	} while(	(inb(0x61) & 0x20) == 0);
	rdtsc(el,eh);
	
	if(count == 0) return -1;	// no counted
	
	// 64bit diff
	diff64bit(el,eh,sl,sh);
	if(eh > 0) return -3;		// too much fast

	clocksperusec10 = (el/TESTTICK)*(TIMER_CLOCK_PER_SEC);
	return clocksperusec10 ;
}
#else

// variant using CMOS second sync 
// UNDER VMWARE REPORTS TRUE CPU CLOCK BUT NOT USEFUL
// FOR OPSYS SINCRONIZATION
uint32 get_cpu_clock()
{
	uint32 sl,sh,el,eh;
	sinc_cmos_second();
	rdtsc(sl,sh);
	sinc_cmos_second(0);
	rdtsc(el,eh);
	
	// 64bit diff
	diff64bit(el,eh,sl,sh);
	if(eh > 0) return -1;		// too much fast
	return el;
}


#endif

