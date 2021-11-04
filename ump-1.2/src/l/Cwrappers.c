/* Wrappers for miscellaneous C functions */
/* DA Gutz 29-Sep-00 */
/* 	$Id: Cwrappers.c,v 1.1 2006/08/16 22:07:58 davegutz Exp $	 */
#ifdef HAVE_CONFIG_H
 #include <config.h>
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdlib.h>   /* for exit */

int Fork()
{
  int childpid = fork();
  if ( childpid == -1 )
    {
      perror("Could not spawn child");
      exit(1);
    }
  return childpid;
}
void Pipe(int fd[2])
{
  if ( pipe(fd) == -1 )
    {
      perror("Could not create pipe");
      exit(1);
    }
}
void Dup2(int source, int destination)
{
  if ( dup2(source, destination) == -1 )
    {
      perror("Could not dup file descriptor");
      exit(1);
    }
}
void Close(int fd)
{
  if ( close(fd) == -1 )
    {
      perror("Could not close file descriptor");
      exit(1);
    }
}
void Setpgid(pid_t pid, pid_t pgid)
{
  if ( -1 == setpgid(pid, pgid) )
    {
      perror("Could not set process group id");
      exit(1);
    }
}
int Shmget(key_t key, size_t size, int flags)
{
  int		rc;
  if ( -1 == (rc = shmget(key, size, flags)) )
    perror("shmget error");
  return(rc);
}
void *Shmat(int id, const void *shmaddr, int flags)
{
  void	*ptr;
  if ( (void *) -1 == (ptr = shmat(id, shmaddr, flags)) )
    perror("shmat error");
  return(ptr);
}
void Shmdt(const void *shmaddr)
{
  if ( shmdt(shmaddr) == -1 )
    perror("shmdt error");
}
void Shmctl(int id, int cmd, struct shmid_ds *buff)
{
  if ( shmctl(id, cmd, buff) == -1 )
    perror("shmctl error");
}
void  Sigaction(int signum,  const  struct  sigaction  *act,
		struct sigaction *oldact)
{
  if ( -1 == sigaction(signum, act, oldact) )
    {
      perror("Warning, could not perform sigaction");
    }
}
void  Sigprocmask(int  how,  const  sigset_t *set, sigset_t
		  *oldset)
{
  if ( -1 == sigprocmask(how, set, oldset) )
    {
      perror("Could not perform procmask operation");
      exit(1);
    }
}
void Execvp(char *file, char **argv)
{
  if ( execvp(file, &argv[0]) == -1 )
    {
      perror("Could not execvp");
      exit(1);
    }
}




