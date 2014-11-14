extern void videocpy(int c, int r, const char *sz);


void amain()
{
	videocpy(0,0,"PIT:Hello from GCC in PM. System Halted");
	while(1) ;
}