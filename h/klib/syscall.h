#ifndef SYSCALL_H
#define SYSCALL_H

// Dichiarazioni prese da SYSCALL.ASM
// utilizzate in sistema.cpp
#define SYSCALLTIPO 0x30

extern "C"{ 
	void syscalle();
	void carica_stato();
	extern int syscall_params[100];
	extern int syscall_table[100];
}

#endif