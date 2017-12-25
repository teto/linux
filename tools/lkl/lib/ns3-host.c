#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <lkl_host.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <model/dce-stdio.h>
#include <model/dce-stdlib.h> /* for dce_malloc/free */
#include <model/dce-pthread.h> /* for pthread */

/* for now rely on mutex since dce has everything available
 * _POSIX_SEMAPHORES */
struct lkl_sem {
#ifdef _POSIX_SEMAPHORES
	sem_t sem;
#else
	pthread_mutex_t lock;
	int count;
	pthread_cond_t cond;
#endif /* _POSIX_SEMAPHORES */
};

/* lkl_thread_t */
struct lkl_tls_key {
	pthread_key_t key;
};

struct lkl_mutex {
	pthread_mutex_t mutex;
};

#define WARN_UNLESS(exp) do {						\
		if (exp < 0)						\
			lkl_printf("%s: %s\n", #exp, strerror(errno));	\
	} while (0)

/* struct SimTask { */
/* 	struct task_struct kernel_task; */
/* 	void *private; */
/* }; */

/* extern struct SimTask *sim_task_create(void *private, unsigned long pid); */

/* pthread_* functions use the reverse convention */
#define WARN_PTHREAD(exp) _warn_pthread(exp, #exp)

static struct lkl_sem *sem_alloc(int count)
{
	struct lkl_sem *sem;

	sem = malloc(sizeof(*sem));
	if (!sem)
		return NULL;

#ifdef _POSIX_SEMAPHORES
	if (sem_init(&sem->sem, SHARE_SEM, count) < 0) {
		lkl_printf("sem_init: %s\n", strerror(errno));
		free(sem);
		return NULL;
	}
#else
	pthread_mutex_init(&sem->lock, NULL);
	sem->count = count;
	WARN_PTHREAD(pthread_cond_init(&sem->cond, NULL));
#endif /* _POSIX_SEMAPHORES */

	return sem;
}

static void sem_free(struct lkl_sem *sem)
{
#ifdef _POSIX_SEMAPHORES
	WARN_UNLESS(sem_destroy(&sem->sem));
#else
	WARN_PTHREAD(pthread_cond_destroy(&sem->cond));
	WARN_PTHREAD(pthread_mutex_destroy(&sem->lock));
#endif /* _POSIX_SEMAPHORES */
	free(sem);
}

static void sem_up(struct lkl_sem *sem)
{
#ifdef _POSIX_SEMAPHORES
	WARN_UNLESS(sem_post(&sem->sem));
#else
	WARN_PTHREAD(pthread_mutex_lock(&sem->lock));
	sem->count++;
	if (sem->count > 0)
		WARN_PTHREAD(pthread_cond_signal(&sem->cond));
	WARN_PTHREAD(pthread_mutex_unlock(&sem->lock));
#endif /* _POSIX_SEMAPHORES */

}

static void sem_down(struct lkl_sem *sem)
{
#ifdef _POSIX_SEMAPHORES
	int err;

	do {
		err = sem_wait(&sem->sem);
	} while (err < 0 && errno == EINTR);
	if (err < 0 && errno != EINTR)
		lkl_printf("sem_wait: %s\n", strerror(errno));
#else
	WARN_PTHREAD(pthread_mutex_lock(&sem->lock));
	while (sem->count <= 0)
		WARN_PTHREAD(pthread_cond_wait(&sem->cond, &sem->lock));
	sem->count--;
	WARN_PTHREAD(pthread_mutex_unlock(&sem->lock));
#endif /* _POSIX_SEMAPHORES */
}

static struct lkl_mutex *mutex_alloc(int recursive)
{
	struct lkl_mutex *_mutex = malloc(sizeof(struct lkl_mutex));
	pthread_mutex_t *mutex = NULL;
	pthread_mutexattr_t attr;

	if (!_mutex)
		return NULL;

	mutex = &_mutex->mutex;
	WARN_PTHREAD(pthread_mutexattr_init(&attr));

	/* PTHREAD_MUTEX_ERRORCHECK is *very* useful for debugging,
	 * but has some overhead, so we provide an option to turn it
	 * off. */
#ifdef DEBUG
	if (!recursive)
		WARN_PTHREAD(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK));
#endif /* DEBUG */

	if (recursive)
		WARN_PTHREAD(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE));

	WARN_PTHREAD(pthread_mutex_init(mutex, &attr));

	return _mutex;
}

static void mutex_lock(struct lkl_mutex *mutex)
{
	WARN_PTHREAD(pthread_mutex_lock(&mutex->mutex));
}

static void mutex_unlock(struct lkl_mutex *_mutex)
{
	pthread_mutex_t *mutex = &_mutex->mutex;
	WARN_PTHREAD(pthread_mutex_unlock(mutex));
}

static void mutex_free(struct lkl_mutex *_mutex)
{
	pthread_mutex_t *mutex = &_mutex->mutex;
	WARN_PTHREAD(pthread_mutex_destroy(mutex));
	free(_mutex);
}

static void thread_detach(void)
{
	// NOP
}

static void thread_exit(void)
{
	// NOP
}

static int thread_join(lkl_thread_t tid)
{
	// NOP
}

static int thread_equal(lkl_thread_t a, lkl_thread_t b)
{
	// NOP
}

static struct lkl_tls_key* tls_alloc(void (*destructor)(void *))
{
	// NOP
	return NULL;
}

static void tls_free(struct lkl_tls_key *key)
{
	// NOP
}

static int tls_set(struct lkl_tls_key *key, void *data)
{
	// NOP
	return 0;
}

static void* tls_get(struct lkl_tls_key *key)
{
	// NOP
	return NULL;
}

//static void* mem_alloc(unsigned long count)
//{
//	// NOP
//	return sim_malloc(count);
//}

//static void mem_free(void *addr)
//{
//	// NOP
//}

static unsigned long long time(void)
{
	// NOP
	return 0;
}

static void* timer_alloc(void (*fn)(void *), void *arg)
{
	// NOP
	
}

static int timer_set_oneshot(void *timer, unsigned long delta)
{
	// NOP
}

static void timer_free(void *timer)
{
	// NOP
}

static void* lkl_ioremap(long addr, int size)
{
	// NOP
}

static int lkl_iomem_access(const __volatile__ void *addr, void *val, int size,
		int write)
{
	// NOP
}

static long gettid(void)
{
	// NOP
}

/* static void jmp_buf_set(struct lkl_jmp_buf *jmpb, void (*f)(void)) */
/* { */
/* 	// NOP */
/* } */

/* static void jmp_buf_longjmp(struct lkl_jmp_buf *jmpb, int val) */
/* { */
/* 	// NOP */
/* } */

/* look at posix host for some understanding */
struct lkl_host_operations lkl_host_ops = {
	.print = dce_printf,
	/* for now a paste of dce_abort */
	.panic = dce_panic,

	.thread_create = dce_pthread_create,
	.thread_detach = dce_pthread_detach,
	.thread_exit = dce_pthread_exit,
	.thread_join = dce_pthread_join,
	.thread_self = dce_pthread_self,
	.thread_equal = dce_pthread_equal,

	.sem_alloc = sem_alloc,
	.sem_free = sem_free,
	.sem_up = sem_up,
	.sem_down = sem_down,

	.mutex_alloc = dce_pthread_mutex_init,
	.mutex_free = mutex_free,
	.mutex_lock = mutex_lock,
	.mutex_unlock = mutex_unlock,

	.tls_alloc = tls_alloc,
	.tls_free = tls_free,
	.tls_set = tls_set,
	.tls_get = tls_get,

	.time = time,
	.timer_alloc = timer_alloc,
	.timer_set_oneshot = timer_set_oneshot,
	.timer_free = timer_free,

	.mem_alloc = dce_malloc,
	.mem_free = dce_free,

	.ioremap = lkl_ioremap,
	.iomem_access = lkl_iomem_access,
	.virtio_devices = lkl_virtio_devs,

  /* @gettid - returns the host thread id of the caller, which need not */
  /* be the same as the handle returned by thread_create */
	.gettid = gettid,

	/* pthread_fiber_manager does some things
	 * maybe try to reestablish the single codepath */
	.jmp_buf_set = 0,
	.jmp_buf_longjmp = 0,  /* jmp_buf_longjmp */
};
