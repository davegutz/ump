/* Wrappers for miscellaneous C functions */
/* DA Gutz 29-Sep-00 */

#ifndef Cwrappthread_h
#define Cwrappthread_h 1

#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include <errno.h>

void  Pthread_attr_destroy(pthread_attr_t *);
void  Pthread_attr_getschedparam(const pthread_attr_t *attr,
				struct sched_param *param);
void  Pthread_attr_init(pthread_attr_t *);
void  Pthread_attr_setdetachstate(pthread_attr_t *, int);
void  Pthread_attr_setschedparam(pthread_attr_t *attr,
				 const struct sched_param *param);
void  Pthread_attr_setscope(pthread_attr_t *, int);
void  Pthread_create(pthread_t *, const pthread_attr_t *,
		     void * (*)(void *), void *);
void  Pthread_condattr_init(pthread_condattr_t *);
void  Pthread_condattr_destroy(pthread_condattr_t *);
void  Pthread_condattr_setpshared(pthread_condattr_t *, int);
void  Pthread_cond_broadcast(pthread_cond_t *);
void  Pthread_cond_signal(pthread_cond_t *);
void  Pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *,
			     const struct timespec *);
void  Pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
void  Pthread_detach(pthread_t);
void  Pthread_getschedparam(pthread_t target_thread, int *policy,
			    struct sched_param *param);
void  Pthread_join(pthread_t, void **);
void  Pthread_key_create(pthread_key_t *, void (*)(void *));
void  Pthread_kill(pthread_t, int);
void  Pthread_mutexattr_init(pthread_mutexattr_t *);
void  Pthread_mutexattr_destroy(pthread_mutexattr_t *);
void  Pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
void  Pthread_once(pthread_once_t *, void (*)(void));
void  Pthread_mutex_init(pthread_mutex_t *, pthread_mutexattr_t *);
void  Pthread_mutex_destroy(pthread_mutex_t *);
void  Pthread_mutex_lock(pthread_mutex_t *);
void  Pthread_mutex_unlock(pthread_mutex_t *);
void  Pthread_setcancelstate(int, int *);
void  Pthread_setschedparam(pthread_t target_thread, int policy,
			    const struct sched_param *param);
void  Pthread_setspecific(pthread_key_t, const void *);

/* not portable
long  pr_thread_id(pthread_t *);
*/

#endif // Cwrappthread_h



