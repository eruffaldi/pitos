
void videocpy(int c, int r, const char *sz)
{
	short * p = (short*)(0xb8000+160*r+2*c);
	while(*sz != 0)
	{
		if(*sz == '\n') break;
		*p++ = (*sz++ & 0xFF)| 0x700;
	}	
}