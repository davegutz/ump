/* Stevens' pthreads wrapper functions. */
/* DA Gutz 17-Jun-01 */
/* 	$Id: Cwrappthread.c,v 1.1 2007/12/16 22:15:50 davegutz Exp davegutz $	 */
#ifdef HAVE_CONFIG_H
 #include <config.h>
#endif
#include "Cerror.h"
#include "Cwrappthread.h"
#include <stdio.h>       /* perror */

void  Pthread_attr_getschedparam(const pthread_attr_t *attr,
				 struct sched_param *param)
{
  int n;
  
  if ( (n = pthread_attr_getschedparam(attr, param)) == 0 )
    return;
  errno = n;
  err_sys("pthread_attr_getschedparam error");
}

void Pthread_attr_init(pthread_attr_t *attr)
{
  int n;
  
  if ( (n = pthread_attr_init(attr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_attr_init error");
}

void Pthread_attr_destroy(pthread_attr_t *attr)
{
  int n;
  
  if ( (n = pthread_attr_destroy(attr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_attr_destroy error");
}

void Pthread_attr_setdetachstate(pthread_attr_t *attr, int detach)
{
  int  n;

  if ( (n = pthread_attr_setdetachstate(attr, detach)) == 0 )
    return;
  errno = n;
  err_sys("pthread_attr_setdetachstate error");
}

void  Pthread_attr_setschedparam(pthread_attr_t *attr,
				 const struct sched_param *param)
{
  int n;
  
  if ( (n = pthread_attr_setschedparam(attr, param)) == 0 )
    return;
  errno = n;
  err_sys("pthread_attr_setschedparam error");
}

void Pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
  int n;

  if ( (n = pthread_attr_setscope(attr, scope)) == 0 )
    return;
  errno = n;
  err_sys("pthread_attr_setscope error");
}

void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,
		    void * (*func)(void *), void *arg)
{
  int n;

  if ( (n = pthread_create(tid, attr, func, arg)) == 0 )
    return;
  errno = n;
  err_sys("pthread_create error");
}

void Pthread_getschedparam(pthread_t target_thread, int *policy,
			   struct sched_param *param)
{
  int n;
  
  if ( (n = pthread_getschedparam(target_thread, policy, param)) == 0 )
    return;
  errno = n;
  err_sys("pthread_getschedparam error");
}

void Pthread_join(pthread_t tid, void **status)
{
  int n;

  if ( (n = pthread_join(tid, status)) == 0 )
    return;
  errno = n;
  err_sys("pthread_join error");
}

void Pthread_detach(pthread_t tid)
{
  int n;

  if ( (n = pthread_detach(tid)) == 0 )
    return;
  errno = n;
  err_sys("pthread_detach error");
}

void Pthread_kill(pthread_t tid, int signo)
{
  int n;
  if ( (n = pthread_kill(tid, signo)) == 0 )
    return;
  errno = n;
  err_sys("pthread_kill error");
}

void Pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
  int n;

  if ( (n = pthread_mutexattr_init(attr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_mutexattr_init error");
}

void Pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
  int n;

  if ( (n = pthread_mutexattr_destroy(attr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_mutexattr_destroy error");
}

#ifdef	_POSIX_THREAD_PROCESS_SHARED
void Pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int flag)
{
  int n; 

  if ( (n = pthread_mutexattr_setpshared(attr, flag)) == 0 )
    return;
  errno = n;
  err_sys("pthread_mutexattr_setpshared error");
}
#endif

void Pthread_mutex_init(pthread_mutex_t *mptr, pthread_mutexattr_t *attr)
{
  int n;

  if ( (n = pthread_mutex_init(mptr, attr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_mutex_init error");
}

void Pthread_mutex_destroy(pthread_mutex_t *mptr)
{
  int n;

  if ( (n = pthread_mutex_destroy(mptr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_mutex_destroy error");
}

void Pthread_mutex_lock(pthread_mutex_t *mptr)
{
  int n;

  if ( (n = pthread_mutex_lock(mptr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_mutex_lock error");
}

void Pthread_mutex_unlock(pthread_mutex_t *mptr)
{
  int n;

  if ( (n = pthread_mutex_unlock(mptr)) == 0 )
    return;
  errno = n;
  perror("pthread_mutex_unlock error");
  return;
}

void Pthread_condattr_init(pthread_condattr_t *attr)
{
  int n;

  if ( (n = pthread_condattr_init(attr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_condattr_init error");
}

void Pthread_condattr_destroy(pthread_condattr_t *attr)
{
  int  n;

  if ( (n = pthread_condattr_destroy(attr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_condattr_destroy error");
}

#ifdef	_POSIX_THREAD_PROCESS_SHARED
void Pthread_condattr_setpshared(pthread_condattr_t *attr, int flag)
{
  int n;

  if ( (n = pthread_condattr_setpshared(attr, flag)) == 0 )
    return;
  errno = n;
  err_sys("pthread_condattr_setpshared error");
}
#endif

void Pthread_cond_broadcast(pthread_cond_t *cptr)
{
  int n;

  if ( (n = pthread_cond_broadcast(cptr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_cond_broadcast error");
}

void Pthread_cond_signal(pthread_cond_t *cptr)
{
  int n;

  if ( (n = pthread_cond_signal(cptr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_cond_signal error");
}

void Pthread_cond_wait(pthread_cond_t *cptr, pthread_mutex_t *mptr)
{
  int n;

  if ( (n = pthread_cond_wait(cptr, mptr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_cond_wait error");
}

void Pthread_cond_timedwait(pthread_cond_t *cptr, pthread_mutex_t *mptr,
			    const struct timespec *tsptr)
{
  int n;

  if ( (n = pthread_cond_timedwait(cptr, mptr, tsptr)) == 0 )
    return;
  errno = n;
  err_sys("pthread_cond_timedwait error");
}

void Pthread_once(pthread_once_t *ptr, void (*func)(void))
{
  int n;

  if ( (n = pthread_once(ptr, func)) == 0 )
    return;
  errno = n;
  err_sys("pthread_once error");
}

void Pthread_key_create(pthread_key_t *key, void (*func)(void *))
{
  int n;

  if ( (n = pthread_key_create(key, func)) == 0 )
    return;
  errno = n;
  err_sys("pthread_key_create error");
}

void Pthread_setcancelstate(int state, int *oldstate)
{
  int n;

  if ( (n = pthread_setcancelstate(state, oldstate)) == 0 )
    return;
  errno = n;
  err_sys("pthread_setcancelstate error");
}

void Pthread_setschedparam(pthread_t target_thread, int policy,
			   const struct sched_param *param)
{
  int n;
  
  if ( (n = pthread_setschedparam(target_thread, policy, param)) == 0 )
    return;
  errno = n;
  err_sys("pthread_setschedparam error");
}

void Pthread_setspecific(pthread_key_t key, const void *value)
{
  int n;

  if ( (n = pthread_setspecific(key, value)) == 0 )
    return;
  errno = n;
  err_sys("pthread_setspecific error");
}

/* NULL not portable */
#if 0
long pr_thread_id(pthread_t *ptr)
{
#if defined(sun)
  return((ptr == NULL) ? pthread_self() : *ptr);	/* Solaris */

#elif defined(__osf__) && defined(__alpha)
  pthread_t	tid;

  tid = (ptr == NULL) ? pthread_self() : *ptr;	/* Digital Unix */
  return(pthread_getsequence_np(tid));
#else
  /* 4everything else */
  return((ptr == NULL) ? pthread_self() : *ptr);
#endif
}
#endif /* if 0 */
