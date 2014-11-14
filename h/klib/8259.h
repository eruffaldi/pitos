#ifndef i8259_H
#define i8259_H

#ifdef __cplusplus 
extern "C" {
#endif

enum IntController {master,slave,none};
void i8259_init();
void eoi(int c);
void i8259_mask(int irq);
void i8259_unmask(int irq);

#ifdef __cplusplus 
}
#endif

#endif 