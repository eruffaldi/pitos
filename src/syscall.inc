//; System Call Declarations...
//; we have just one table included
//; more times and extracting things upon usage...
//;
//; 1) in utente.s we create the stub
//; 2) in syscall.s we extract the table of addresses and parameter sizes

SYSCALL activate_p, 5,0
SYSCALL terminate_p, 0,1
SYSCALL sem_ini, 3,2
SYSCALL sem_wait, 1,3
SYSCALL sem_signal,1,4
SYSCALL sysGetOsName,2,5
SYSCALL get_tick,2,6
SYSCALL delay,1,7
SYSCALL get_pri,1,8
SYSCALL get_basepri,1,9
SYSCALL get_pid,1,10
SYSCALL signal,2,11
SYSCALL raise,1,12
SYSCALL __signalrespin,0,13
SYSCALL print,2,14
SYSCALL free,1,15
SYSCALL _malloc,1,16

