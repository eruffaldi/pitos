// MEMORY INFO
// 
// Memory Block
// 		# blocks
//		free memory if not ACPI
//		BLOCK 1 = (low base,hi base, low size, hi size, type 1 = free, 2 = reserved ...)
//		BLOCK 2
// Required: pmload3.asm
#include "mypc.h"
#include "mylib.h"
#include <stdio.h>

#define STACKBASE 0x70000
#define LOADADDRESS 0x30000

enum BlockType { M_FREE, M_ALLOCATED, M_OSRES, M_BIOSRES, M_VMPAGE};
#define MAXBLOCK 20

// physical memory blocks
struct memblock {
	uint32 base;
	uint32 length;
	uint32 type;	// BlockType
};

memblock blocks[MAXBLOCK];
uint32 total_phys_mem;

void mem_add_block(uint32 base, uint32 len, uint32 type)
{
	// find a free block or split an existing one (one or more)...
}
// REMEME


int ir = 0;
void testm()
{
	char buf[200];
	int i;
	uint32 * p = (uint32*)0x20000;
	
	
	if(*p == 0) {
		sprintf(buf,"Free Memory Is %d", p[1]);
		videocpy(0,ir++,buf);
	}
	else {
		int n = p[0];
		p += 2;
		for(i = 0; i < n; i++,p+= 5) {
			sprintf(buf,"Memory Block %8x%8x L %20d = %d", 
				p[1],p[0], p[2], p[4]);				
			videocpy(0,ir++, buf);
		}
	}
}

void initm()
{
	// 1) use memory info 
	// 2) add reserved memory: videomemory B0000 + 8*160*25
	//    loaded 30000 and base stack STACKBASE + 64K (arbitrary)
	//    add vm pages (
	uint32 * p = (uint32*)0x20000;
		
	if(*p == 0) {
		total_phys_mem = p[1];
		mem_add_block(0, total_phys_mem, M_FREE);	
	}
	else {
		int n = p[0];
		p += 2;
		total_phys_mem = 0;
		for(int i = 0; i < n; i++,p+= 5) {
			mem_add_block(p[0], p[2],p[4] == 2 ? M_BIOSRES : M_FREE);
			total_phys_mem += p[2];
		}
		// calcolare total_phys_mem ...

	}

	// start adding memory areas
	p = (uint32*)LOADADDRESS;	
	mem_add_block(LOADADDRESS, p[0], M_OSRES);
	mem_add_block(0xB800, 25*160*8, M_BIOSRES);
	mem_add_block(STACKBASE, 65536, M_OSRES);
	// add VM
	mem_add_block(0, 4096*((total_phys_mem>>22)+1),M_VMPAGE);
	
}

void dumpm()
{
	int i;
	for( i = 0; i < sizeof(blocks)/sizeof(blocks[0]); i++)
	{
		char buf[200];
		sprintf(buf, "Block %x L %d T %d",
			blocks[i].base, blocks[i].length, blocks[i].type);
		videocpy(0,ir++, buf);
	}
}

void cmain()
{
	videocpy(0,ir++,"Getting Memory Info");
	testm();
	initm();
	dumpm();
	while(1);
}

