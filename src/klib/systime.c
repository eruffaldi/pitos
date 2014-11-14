// SYSTIME.C
// 
// Timing Functions:
// la cosa fondamentale che ci interessa e' il numero di millisecondi trascorsi dal 1/1/1970,
// questo va rappresentato su 64 bit. 
// La prima informazione importante e' contenuta dentro al CMOS che puo' essere associata ad un time
// stamp per ottenere un riferimento. Ogni altro calcolo verra' riferito ad essa.
// Avremo cosi':
// 	* boot time, boot stamp, boot millis 
// Da cui ricavare i millis: (now stamp - boot stamp) * STAMP_TO_MILLIS + bootmillis
// 
#include "mypc.h"
#include "mylib.h"
#include "systime.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mc146818.h"

static uint64 boot_time;			// tempo della chiama init_boot_time in secondi
static uint64 boot_stamp;			// espresso come TSC
static uint64 boot_millis;			// espresso come millisecondi
static uint32 STAMP_TO_MILLIS;		// result = 2^32 / (TSC per usec)


uint64 get_current_millis()
{
	uint64 diff = get_tick_count()-boot_stamp;
	return ((diff*STAMP_TO_MILLIS)>>32) + boot_millis;
}

uint64 get_tick_count()
{
	uint64 r;
	rdtscll(r);
	return r;
}

uint64 get_boot_millis()
{
	return boot_millis;
}

void init_boot_time()
{
	// cpu time is clocks per each 10*usec
	uint64 t = 1000;
	t <<= 32;
	STAMP_TO_MILLIS  = t/get_cpu_clock();	

	boot_time = get_cmos_time(0);
	boot_stamp = get_tick_count();
	boot_millis = boot_time*1000;
}


static inline uint64 mktime(unsigned int year, unsigned int mon,
         unsigned int day, unsigned int hour,
         unsigned int min, unsigned int sec)
{
	uint64 r;
         if (0 >= (int) (mon -= 2)) {    /* 1..12 -> 11,12,1..10 */
                 mon += 12;      /* Puts Feb last since it has leap day */
                 year -= 1;
         }
         
         r = year/4 - year/100+year/400 + 367*mon/12+day;
         r += year*365 - 719499;
         r *= 24;
         r += hour;
         r *= 60;
         r += min;
         r *= 60;
         r += sec;
         return r;
#if 0
         return (((
			(uint32)(year/4 - year/100 + year/400 + 367*mon/12 + day) +
				year*365 - 719499
			)*24 + hour /* now have hours */
			)*60 + min /* now have minutes */
			)*60 + sec; /* finally seconds */
#endif			
}

// sinc with cmose next second update
void sinc_cmos_second()
{
	int i;
	for (i = 0 ; i < 1000000 ; i++)	/* may take up to 1 second... */
		if (CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP)
			break;
	for (i = 0 ; i < 1000000 ; i++)	/* must try at least 2.228 ms */
		if (!(CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP))
			break;
}

// get system time from CMOS
// From: Linux 
uint64 get_cmos_time(struct tm*ptm)
{
	unsigned int year, mon, day, hour, min, sec;
	struct tm atm;
	int i;

	/* The Linux interpretation of the CMOS clock register contents:
	 * When the Update-In-Progress (UIP) flag goes from 1 to 0, the
	 * RTC registers show the second which has precisely just started.
	 * Let's hope other operating systems interpret the RTC the same way.
	 */
	/* read RTC exactly on falling edge of update flag */
	for (i = 0 ; i < 1000000 ; i++)	/* may take up to 1 second... */
		if (CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP)
			break;
	for (i = 0 ; i < 1000000 ; i++)	/* must try at least 2.228 ms */
		if (!(CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP))
			break;
	do { /* Isn't this overkill ? UIP above should guarantee consistency */
		sec = CMOS_READ(RTC_SECONDS);
		min = CMOS_READ(RTC_MINUTES);
		hour = CMOS_READ(RTC_HOURS);
		day = CMOS_READ(RTC_DAY_OF_MONTH);
		mon = CMOS_READ(RTC_MONTH);
		year = CMOS_READ(RTC_YEAR);
	} while (sec != CMOS_READ(RTC_SECONDS));
	if ((!(CMOS_READ(RTC_CONTROL) & RTC_DM_BINARY) || RTC_ALWAYS_BCD))
	  {
	    BCD_TO_BIN(sec);
	    BCD_TO_BIN(min);
	    BCD_TO_BIN(hour);
	    BCD_TO_BIN(day);
	    BCD_TO_BIN(mon);
	    BCD_TO_BIN(year);
	  }
	if (year  < 70)
		year += 100;
		
	if(ptm) {
	atm.tm_sec = sec;
	atm.tm_min = min;
	atm.tm_hour = hour+9;		
	atm.tm_mon = mon;
	atm.tm_mday = day;
	atm.tm_year = year;
		*ptm = atm;
	}	
	return mktime(year, mon, day, hour, min, sec);	
}


