#ifndef SISTEMAG_H
#define SISTEMAG_H

#ifdef __cplusplus
extern "C" {

#endif
#include "tprocess.h"

extern void ki_activate_p(void (*f)(PROCESSPARAM h), PROCESSPARAM h, int prec, HPROCESS & idx, bool & risu);
#ifdef __cplusplus
}
extern void kernel_init();
extern void kernel_go();
#endif

#endif