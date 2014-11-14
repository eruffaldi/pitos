// Memory Management Unit 
// by Emanuele Ruffaldi 2001
//
// Let you use and manage memory: you have virtual memory and physical memory, each process can
// have a different address space...
// 1) initialize
// 2) 
#ifndef MMU_H
#define MMU_H

typedef uint32 AS;

void mmu_init();
void mmu_as_bind(AS as, DWORD phaddr,DWORD laddr, DWORD size);
AS mmu_as_current();
AS mmu_as_create();
void mmu_as_set(HPROCESS hproc, AS as);
#endif