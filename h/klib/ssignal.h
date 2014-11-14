#ifndef SYSSIGNAL_H
#define SYSSIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif 

// system side signal
// required is signal.h
typedef unsigned int signal_mask_t;

typedef void (*sigfunc)(int);

struct signal_t {
	sigfunc	  handler;
};

#define SIGCOUNT	32
extern int	ki_raise(int _sig);
extern void	(*ki_signal (int _sig, void (*_func)(int)))(int);

#ifdef __cplusplus
}
#endif 
#endif