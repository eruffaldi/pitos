
// got some bug with the VMWARE!!
// with the first and the last positions
#define VIDEOMEM 0x000B8000

void videocpy(int c, int r, const char *sz)
{
	short * p = (short*)(VIDEOMEM+160*r+2*c);
	while(*sz != 0)
	{
		*p++ = (*sz++ | 0x700);
	}
}

void amain()
{
	const char * sz = "Hello from C in PM";	
	int i;
	
	for(i = 0;  i <79; i++) 
		videocpy(i,0,"*"),videocpy(i,24,"*");
	for(i = 0;  i <24; i++) 
		videocpy(0,i,"*"),videocpy(79,i,"*");
		
	videocpy(5,5,sz);


}
