#include <mylib.h>

// Video Support Routines
#define VIDEOMEM 0x000B8000

int videoport_reg,videoport_val;
int start = VIDEOMEM;
int currentPage = 0;



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

void video_clear()
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

