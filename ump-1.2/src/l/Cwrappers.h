/* Wrappers for miscellaneous C functions */
/* 	$Id: Cwrappers.h,v 1.1 2006/08/16 22:07:28 davegutz Exp $	 */
#ifndef Cwrappers_h
#define Cwrappers_h 1

#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <pthread.h>
#include "Cwrappthread.h"

int   Fork();
void  Pipe(int fd[2]);
void  Dup2(int source, int destination);
void  Close(int fd);
int   Shmget(key_t, size_t, int);
void *Shmat(int, const void *, int);
void  Shmdt(const void *);
void  Shmctl(int, int, struct shmid_ds *);
void  Execvp(char *file, char **argv);
void  Setpgid(pid_t pid, pid_t pgid);
void  Sigaction(int signum,  const  struct  sigaction  *act,
       struct sigaction *oldact);
void  Sigprocmask(int  how,  const  sigset_t *set, sigset_t *oldset);

#endif // Cwrappers_h
