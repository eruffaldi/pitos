#include <stdio.h>
#include <time.h>

void main(int argc, char*argv[])
{
	struct tm * ptm;
	int k;
	struct tm atm;
	if(argc < 2) return;
	k = atoi(argv[1]);
	printf("%d\n", k);	
	ptm = localtime(&k);
	atm = *ptm;
	
	printf("DATE IS %2d/%2d/%4d %d.%d:%d\n", atm.tm_mday,atm.tm_mon+1,atm.tm_year+1900,
		atm.tm_hour,atm.tm_min,atm.tm_sec);
	
	k = time(0);
ptm = localtime(&k);
	atm = *ptm;
printf("DATE IS %2d/%2d/%4d %d.%d:%d", atm.tm_mday,atm.tm_mon+1,atm.tm_year+1900,
		atm.tm_hour,atm.tm_min,atm.tm_sec);
			
}