
static void videocpy(int c, int r, const char *sz)
{
	short * p = (short*)(0xB8000+160*r+2*c);
	while(*sz != 0)
	{
		if(*sz == '\n') break;
		*p++ = (*sz++ & 0xFF)| 0x700;
	}	
}

void amain()
{
	videocpy(0,0,"PIT:Hello from GCC in PM. System Halted");
	while(1) ;
}

