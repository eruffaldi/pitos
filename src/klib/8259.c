#include "mylib.h"
#include "8259.h"


#define IRQ0M	0x20
#define IRQ0S	0x28
#define EOI 	0x20
#define MASTER	0x20
#define SLAVE   0xA0

void i8259_init()
{	
	outb(0x21, 0xFF);			// mask	
	outb(0xA1, 0xFF);			// mask	
	
	outb(0x20,0x11);		// initialization sequence
	outb(0x21, IRQ0M);		// 0x20
	outb(0x21, 4);			// slave is on pin 2
	outb(0x21, 1);			// nested mode with EOI

	outb(0xA0,0x11);		// initialization sequence
	outb(0xA1, IRQ0S);		// 0x28
	outb(0xA1, 2);			// slave id is 2
	outb(0xA1, 1);			// nested mode
	//udelay(100);			// give time
	
	outb(0xA1, 0xFF);			// all masked
	outb(0x21, 0xFF);			// all masked
}

void i8259_mask(int irq)
{
	int port = irq > 8 ? SLAVE+1 : MASTER+1;
	outb(port, inb(port)| (1 << (irq & 7)));
}

void i8259_unmask(int irq)
{
	int port = irq > 8 ? SLAVE+1 : MASTER+1;
	outb(port, inb(port) & ~(1 << (irq & 7)));
}

void eoi(int c)
{
	if(c != none) 
		outb(c == master ? MASTER : SLAVE,EOI);	// send eoi
}