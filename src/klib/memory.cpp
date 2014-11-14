// Memory Management Functions
// Allocation Management, Simplified
#include <mylib.h>

static int baseAddress = 10000000;
static unsigned length = 512000;
static unsigned allocated = 0;

extern "C"
char * sbrk(unsigned size)
{	
	int r = baseAddress;
	if(allocated+size > length) {
		// send a signal!!!
		videocpy(0,1,"************** OUT OF MEMORY *********************");
		while(1);
		return 0;
	}
	baseAddress+= size;
	allocated += size;
	return (char*)r;
}

// C++ SUPPORT ROUTINES
extern "C"
void * __builtin_new(unsigned n)
{
	return ki_malloc(n);
}

extern "C"
void __builtin_delete(void*p)
{
	ki_free(p);
}

extern "C"
void *__builtin_vec_new(int n)
{
	return ki_malloc(n);
}

extern "C"
void __builtin_vec_delete(void*p)
{
	ki_free(p);
}
