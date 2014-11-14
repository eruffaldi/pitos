/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_Inline_pc_h_
#define __dj_include_Inline_pc_h_

#ifndef INLINEOP
#define INLINEOP __inline__ static
#endif

#ifdef __cplusplus
extern "C" {
#endif

INLINEOP unsigned char
inportb (unsigned short _port)
{
  unsigned char rv;
  __asm__ __volatile__ ("inb %1, %0"
	  : "=a" (rv)
	  : "d" (_port));
  return rv;
}

INLINEOP unsigned short
inportw (unsigned short _port)
{
  unsigned short rv;
  __asm__ __volatile__ ("inw %1, %0"
	  : "=a" (rv)
	  : "d" (_port));
  return rv;
}

INLINEOP unsigned long
inportl (unsigned short _port)
{
  unsigned long rv;
  __asm__ __volatile__ ("inl %1, %0"
	  : "=a" (rv)
	  : "d" (_port));
  return rv;
}

INLINEOP void
outportb (unsigned short _port, unsigned char _data)
{
  __asm__ __volatile__ ("outb %1, %0"
	  :
	  : "d" (_port),
	    "a" (_data));
}

INLINEOP void
outportw (unsigned short _port, unsigned short _data)
{
  __asm__ __volatile__ ("outw %1, %0"
	  :
	  : "d" (_port),
	    "a" (_data));
}

INLINEOP void
outportl (unsigned short _port, unsigned long _data)
{
  __asm__ __volatile__ ("outl %1, %0"
	  :
	  : "d" (_port),
	    "a" (_data));
}

INLINEOP void
jmpjmp()
{
	__asm__ __volatile__ (".word 0x00eb,0x00eb");
}

#define outb(p,d) outportb(p,d)
#define inb(p)	  inportb(p)

#ifdef __cplusplus
}
#endif

#endif
