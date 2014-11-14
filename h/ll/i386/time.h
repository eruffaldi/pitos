/* Project:     OSLib
 * Description: The OS Construction Kit
 * Date:                1.6.2000
 * Idea by:             Luca Abeni & Gerardo Lamastra
 *
 * OSLin is an SO project aimed at developing a common, easy-to-use
 * low-level infrastructure for developing OS kernels and Embedded
 * Applications; it partially derives from the HARTIK project but it
 * currently is independently developed.
 *
 * OSLib is distributed under GPL License, and some of its code has
 * been derived from the Linux kernel source; also some important
 * ideas come from studying the DJGPP go32 extender.
 *
 * We acknowledge the Linux Community, Free Software Foundation,
 * D.J. Delorie and all the other developers who believe in the
 * freedom of software and ideas.
 *
 * For legalese, check out the included GPL license.
 */

/*	A wrapper to the time handling functions	*/

#ifndef __LL_TIME_H__
#define __LL_TIME_H__

#include <ll/sys/types.h>

#ifndef NULL
    #define NULL 0L
#endif

#include <ll/sys/ll/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 65536(tics/hour) / 3600(sec/hour) * 5(scale) = 91.02
   The 5 is to make it a whole number (18.2*5=91) so that
   floating point ops aren't required to use it. */
#define CLOCKS_PER_SEC	91

// #include <sys/djtypes.h>

typedef int clock_t;
typedef int time_t;
struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
  char *__tm_zone;
  int __tm_gmtoff;
};

#if 0
char *		asctime(const struct tm *_tptr);
clock_t		clock(void);
char *		ctime(const time_t *_cal);
double		difftime(time_t _t1, time_t _t0);
struct tm *	gmtime(const time_t *_tod);
struct tm *	localtime(const time_t *_tod);
time_t		mktime(struct tm *_tptr);
size_t		strftime(char *_s, size_t _n, const char *_format, const struct tm *_tptr);
time_t		time(time_t *_tod);

#define CLK_TCK	CLOCKS_PER_SEC

extern char *tzname[2];

void	tzset(void);

#define tm_zone __tm_zone
#define tm_gmtoff __tm_gmtoff

struct timeval {
  time_t tv_sec;
  long tv_usec;
};

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

#include <ll/sys/types.h>

typedef long long uclock_t;
#define UCLOCKS_PER_SEC 1193180

int		gettimeofday(struct timeval *_tp, struct timezone *_tzp);
unsigned long	rawclock(void);
int		select(int _nfds, fd_set *_readfds, fd_set *_writefds, fd_set *_exceptfds, struct timeval *_timeout);
int		settimeofday(struct timeval *_tp, ...);
uclock_t	uclock(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
