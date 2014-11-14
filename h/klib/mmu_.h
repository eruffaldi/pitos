// Internal Memory Management Functions

enum MemoryType { RESERVED, FREE};

// Memory Block Define a Region of Memory
struct MemoryBlock
{
	uint32 begin;
	uint32 size;
	MemoryType type;	
};

// Address Space Implementation:
typedef AddressSpace
{
	uint32 base;		// directory base 4096
	
};