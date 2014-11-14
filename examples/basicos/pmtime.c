#include "mypc.h"
#include "mylib.h"
#include "systime.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define DOIT(x)  (int)(x >> 32), (int)(x), (int)(x)


uint64 calibrate2()
{
	uint64 b,e;
	get_cmos_time(0);
	rdtscll(b);
	get_cmos_time(0);
	rdtscll(e);
	return e-b;
}

void shownow(int r)
{
	char buffer[200];
	struct tm atm;
	uint64 timen,tick;
	unsigned long t;
	

	t = get_cmos_time(&atm);
	sprintf(buffer,"CMOS %2d/%2d/%4d %d.%d:%d = %d seconds", atm.tm_mday,atm.tm_mon,atm.tm_year,
		atm.tm_hour,atm.tm_min,atm.tm_sec, (int)t);
	videocpy(0,r,buffer);	
	timen = get_current_millis();
	tick = get_tick_count();
	
	sprintf(buffer,"Current Time %x:%x %u, Tick %x%x %u", DOIT(timen),DOIT(tick));
	videocpy(0,r+1,buffer);
	
	
}


// Calcolo della Frequenza di CPU: utilizziamo il timer
// senza interruzioni e con polling per stabilire il numero di cicli
// effettuati e quindi la frequenza della cpu
void cmain()
{	
	char buffer[200];
	unsigned long t;
	struct tm atm;
	uint64 timeb,timen,tick,d;
	
	cli();
	video_clear();
	videocpy(1,0,"pmtime shows the time from CMOS & RDTSC, and waits 5 secs");	
	init_boot_time();
	
	{
	uint64 a = calibrate2();
	sprintf(buffer,"MY RDTSC in 1 sec is %x %x %u, cpu time is %d", 
		DOIT(a), get_cpu_clock());
	videocpy(0,10,buffer);
	}
	
	t = get_cmos_time(&atm);
	sprintf(buffer,"CMOS Second from 1/1/1970: %ld", t);
	videocpy(0,1,buffer);
	timeb = get_boot_millis();
	sprintf(buffer,"Boot Time %x:%x %u", DOIT(timeb));
	videocpy(0,2,buffer);	
	shownow(3);
		
	// wait 5 seconds
	timen = get_current_millis();
	while(((d = get_current_millis()) - timen) < 1000*5) {
		sprintf(buffer,"Diff is %d             ", (int)(d-timen));
		videocpy(0,11,buffer);
	}
	shownow(5);
	sti();
}
