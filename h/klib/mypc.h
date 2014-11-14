// My PC Include
// uses DJGPP
// defines: in/out cli/sti
#ifndef _MYPC_H
#define _MYPC_H
#include "pc.h"

#define __GNU__

#define INLINEOP __inline__ static
#ifdef __cplusplus
extern "C" {
#endif

#ifndef STIDEF
#define STIDEF
INLINEOP void cli() {  __asm__  ("cli");}
INLINEOP void sti() {	__asm__ __volatile__ ("sti");}
INLINEOP void halt() { 	__asm__ __volatile__ ("hlt"); }

#endif

INLINEOP void iret() {	__asm__ __volatile__ ("iret");}
#define rdtsc(low,high) __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

#define rdtscl(low) __asm__ __volatile__ ("rdtsc" : "=a" (low) : : "edx")

#define rdtscll(val)      __asm__ __volatile__ ("rdtsc" : "=A" (val))

	
// first -= second
#define diff64bit(l1,h1,l2,h2) 	__asm__ ("subl %2,%0\n\t"	      \
	"sbbl %3,%1"	\
			:"=a" (l1), "=d" (h1)	\
			:"g" (l2), "g" (h2),	\
			 "0" (l1), "1" (h1))

#ifdef __cplusplus
}
#endif

#endif
