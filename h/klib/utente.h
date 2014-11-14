#ifndef UTENTE_H
#define UTENTE_H

// Lato delle Primitive di Sistema viste dall'utente...
#include "tprocess.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus

void activate_p(void (*f)(PROCESSPARAM h), PROCESSPARAM h, int prec, HPROCESS & idx, bool & risu);
void terminate_p();

void sem_init(int valore, HSEM & idx,bool & risu);
void get_tick(int &lo, int & hi);
void get_pri(int &h);
void get_basepri(int &h);
void get_pid(void *&h);
#endif
void sem_signal(HSEM idx);
void sem_wait(HSEM idx);
void sysGetOsName(char * buf, int len);
void delay(int n);
void print(char * buf, int page);

#ifdef __cplusplus
}
#endif

#endif