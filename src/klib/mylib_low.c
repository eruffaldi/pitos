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

int videoport_reg,videoport_val;
int start = 0xB8000;
int currentPage = 0;
int kb_has_callback = 0;
int kb_changed;
int kb_last;
uint32 clocksperusec10;
// got some bug with the VMWARE!!
// with the first and the last positions
#define VIDEOMEM 0x000B8000





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


// video PORT TEST
int video_setpage(int page)
{
	int old = currentPage;
	int offset = page << 11;
	if(page < 0 || page > 7) return currentPage;
	videoport_reg = 0x3d4;
	videoport_val = 0x3d5;
//	cli();
	outportb(videoport_reg, 12);
	outportb(videoport_val, offset >> 8);
	outportb(videoport_reg, 13);
	outportb(videoport_val, offset);		
	start = VIDEOMEM+page * 4096;
	currentPage = page;
//	sti();
	return old;
}

void video_char(int c, int r, char cc)
{
	short * p = (short*)(start+160*r+2*c);
	*p++ = (cc & 0xFF)| 0x700;
}


void videocpy(int c, int r, const char *sz)
{
	short * p = (short*)(start+160*r+2*c);
	while(*sz != 0)
	{
		if(*sz == '\n') break;
		*p++ = (*sz++ & 0xFF)| 0x700;
	}	
}

void	video_clear()
{
	int i = 0;
	short * p = (short*)(start);
	for(i = 0; i < 80*25; i++)
	{
		*p++ = 0x700;
	}
}


int video_getpage() 
{
	return currentPage;
}

void video_eraseline(int r)
{
	int i;
	short * p = (short*)(start+160*r);
	for( i = 0; i < 80; i++)
		*p++ = 0x700;
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
