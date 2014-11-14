#ifndef MYLIB_H
#define MYLIB_H

#include "mypc.h"

// Base Data Types
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef uint16 selector_t;
typedef uint32 offset_t;
typedef unsigned char byte;
typedef byte uint8;
typedef signed char int8;
typedef long long int64;
typedef unsigned long long uint64;

// Utility Functions
#ifdef __cplusplus
extern "C"
{
#endif

extern void videocpy(int c, int r, const char *sz);
// Protected Mode Declarations primitives
typedef struct {
	uint32 l;		// selector, offsetL
	uint32 h;		// offsetH,  PDPL0 ...
} idtEntry;

extern idtEntry idt[256];

void idtSetEntry(int idx, void (*fx)(int h));
void idtSetEntryAsm(int idx, void * fx,int level);


void udelay(unsigned long);

char *
itoa(int value, char *string, int radix);

#pragma pack(1)
typedef struct 
{
	uint16 limitlo;
	uint16 baselo;
	uint8  basemid;
	uint8  access;
	uint8  limithi;		// plus AVL and G
	uint8  basehi;
}gdtdes;

typedef struct 
{
	uint16 offsetlo;
	uint16 selector;
	uint8  res;
	uint8  access;
	uint16 offsethi;
}idtdes;

extern gdtdes gdt[256];

#pragma pack(1)
typedef struct 
{
	int link, ESP0;
	short SS0;
	short fill0;	
	int ESP1,SS1,ESP2,SS2,CR3,EIP, EFLAGS,EAX,ECX,EDX,EBX,ESP,EBP,
	ESI,EDI,ES,CS,SS,DS,FS,GS,LDT,IOMAP;
} myTSS;

extern void ki_free(void*p);
extern void *ki_malloc(int s);


// Video Functions
extern int video_setpage(int);
extern int video_getpage();
extern void video_eraseline(int);
extern void	video_clear();
extern void video_char(int,int,char);
extern int VIDEOROWS;
// Logging
extern void log_message(char * sz);
extern void log_init();
extern void log_show();

// Timer
// tick = 840ns
#define TIMER_CLOCK_PER_SEC	1193180
	extern void timer_start(short tick);
extern void timer_stop();

// Keyboard 
/*
 Interfaccia Tastiera:
	0x60	RBR
	0x64 	STR	bit 0=FI, bit 1=FO
*/
#define KB_RBR 0x60
#define KB_STR 0x64

extern unsigned kb_waitkey();
extern void kb_empty();
extern void kb_setcallback(void (*f)(int h));
extern char * sbrk(unsigned );
extern uint32 get_cpu_clock();
extern myTSS _TSS;
#ifdef __cplusplus
}
#endif


#endif 
