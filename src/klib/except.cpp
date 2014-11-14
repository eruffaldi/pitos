#include <stdio.h>
#include <mylib.h>
#include <sistema.h>
#include <syscall.h>


enum ExceptionType { Any, Fault, Trap, Abort};
struct cpuexception {
	char * desc;
	ExceptionType type;
	int errorCode;
};

extern cpuexception handlerText[];


struct opcode_t {
	uint8 code,mask;
	const char * desc;
};

opcode_t opcodes[] = {
{	0xFA, 0xFF, "cli"},
{	0xE4,0xF6, "in"},
{	0xE6,0xF6, "out"},
{	0xFB,0xFF,"sti"},
{  0xCF,0xFF,"iret"},
{ 0xC2,0xF6,"ret"},
};

const char * findopcode(uint8*);
// exception handling
// the stack is:
// LOW
// 		return handler
//		h
//  ->  esp
//		12 items
//		eip
//		cs 
//   	eflags
//      error code
// TOP
void handlerNoCode(int h, int*esp)
{
	char buf[100];
	log_show();
	sprintf(buf, "EXC %d: %s: %04x:%08x",h,handlerText[h].desc, esp[14],esp[13]);
	log_message(buf);
	
	// INVALID OPCODE: check instruction opcode
	if(h == 6|| h == 13) {
		uint8* p = (uint8*) esp[13];
		const char * found = findopcode(p);
		// cerca opcode

		sprintf(buf, "OPCODE %02x %s",(uint32)*p,found);
		log_message(buf);
	}
	if(esecuzione != 0) {
		ki_terminate_p();
		if(esecuzione != 0)
			carica_stato();
	}
	else
		while(1);
}

const char * findopcode(uint8*p)
{
	uint8 c = *p;
	for(unsigned i = 0; i < sizeof(opcodes)/sizeof(opcodes[0]); i++)
		if((c & opcodes[i].mask) == opcodes[i].code)
			return opcodes[i].desc;
	return "?";
}


typedef void (*TT)(int);

void exceptions_install()
{

	for(int i = 0; i<20; i++)
		if(i != 15)
			idtSetEntry(i,(TT)handlerNoCode);
}


cpuexception handlerText[] =
{
{	"Division By 0", Fault,0},
{	"Debug Exception",Any,0},
{	"NMI",Abort,0},
{	"BreakPoint",Trap,0},
{	"Overflow", Trap,0},
{	"BOUND", Fault,0},
{	"Invalid OPCODE", Fault,0},
{	"Device Not Available", Fault, 0},
{	"Double Fault Exception", Abort,0},
{	"Coprocessor Segment Overrun", Abort,0},
{	"Invalid TSS", Fault, 1},
{	"Segment Not Present", Fault, 1},
{	"Stack Fault Exception", Fault, 1},
{	"General Protection Exception", Fault, 1},
{	"Page Fault", Fault,1},
{	"??", Any,0},
{	"FPU Exception", Fault, 0},
{	"Alignment Check Exception", Fault, 1},
{	"Machine Check Exception", Abort,0},
{	"SIMD FP Exception", Fault,0} 
};
