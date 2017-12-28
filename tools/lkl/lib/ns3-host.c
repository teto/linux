/* Heavily inspired by posix-host.c with some calls
 * prefixed with `dce_` */
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <lkl_host.h>
#include <linux/sched.h>
#include <linux/wait.h>


// for dce-signal.h
typedef void (*sighandler_t)(int);

// todo rename folder to include
#include <model/dce-stdio.h>
#include <model/dce-stdlib.h> /* for dce_malloc/free */
#include <model/dce-pthread.h> /* for pthread */
#include <model/dce-signal.h> /* for panic */
#include <model/dce-time.h> /* for panic */
#include <model/dce-timerfd.h> /* for timerfr_XX */

/* move it to ns3-host ? then we don't know lkl_start_kernel */
/* SimImported alias for lkl_host_operations ? */
extern struct SimImported *g_imported;
extern struct SimKernel *g_kernel;
/* extern struct lkl_host_operations lkl_host_ops; */

// original definition is
/* struct SimDevice { */
/* 	struct net_device dev; */
/* 	void *priv; */
/* }; */
struct SimDevice {
	struct lkl_netdev *dev;
	void *priv;
};

/*
 * dad
 */
struct SimExported {
	/* struct SimTask *(*task_create)(void *priv, unsigned long pid); */
	/* void (*task_destroy)(struct SimTask *task); */
	/* void *(*task_get_private)(struct SimTask *task); */
	/* int (*sock_socket)(int domain, int type, int protocol, */
	/* 		struct SimSocket **socket); */
	/* int (*sock_close)(struct SimSocket *socket); */
	/* ssize_t (*sock_recvmsg)(struct SimSocket *socket, struct msghdr *msg, */
	/* 			int flags); */
	/* ssize_t (*sock_sendmsg)(struct SimSocket *socket, */
	/* 			const struct msghdr *msg, int flags); */
	/* int (*sock_getsockname)(struct SimSocket *socket, */
	/* 			struct sockaddr *name, int *namelen); */
	/* int (*sock_getpeername)(struct SimSocket *socket, */
	/* 			struct sockaddr *name, int *namelen); */
	/* int (*sock_bind)(struct SimSocket *socket, const struct sockaddr *name, */
	/* 		int namelen); */
	/* int (*sock_connect)(struct SimSocket *socket, */
	/* 		const struct sockaddr *name, int namelen, */
	/* 		int flags); */
	/* int (*sock_listen)(struct SimSocket *socket, int backlog); */
	/* int (*sock_shutdown)(struct SimSocket *socket, int how); */
	/* int (*sock_accept)(struct SimSocket *socket, */
	/* 		struct SimSocket **newSocket, int flags); */
	/* int (*sock_ioctl)(struct SimSocket *socket, int request, char *argp); */
	/* int (*sock_setsockopt)(struct SimSocket *socket, int level, */
	/* 		int optname, */
	/* 		const void *optval, int optlen); */
	/* int (*sock_getsockopt)(struct SimSocket *socket, int level, */
	/* 		int optname, */
	/* 		void *optval, int *optlen); */
	/* void (*sock_poll)(struct SimSocket *socket, void *ret); */
	/* void (*sock_pollfreewait)(void *polltable); */

	// we can use instead
  /* lkl_register_netdev_fd */
/* lkl_netdev_add (Must be called before calling lkl_start_kernel) */
/* int lkl_netdev_add(struct lkl_netdev *nd, struct lkl_netdev_args* args); */
/* lkl_netdev_pipe_create */
/* lkl_netdev_raw_create */
	struct SimDevice *(*dev_create)(const char *ifname, void *priv,
					enum SimDevFlags flags);
	void (*dev_destroy)(struct SimDevice *dev);
	void *(*dev_get_private)(struct SimDevice *task);
	void (*dev_set_address)(struct SimDevice *dev,
				unsigned char buffer[6]);
	void (*dev_set_mtu)(struct SimDevice *dev, int mtu);
	struct SimDevicePacket (*dev_create_packet)(struct SimDevice *dev,
						int size);
	void (*dev_rx)(struct SimDevice *dev, struct SimDevicePacket packet);

	void (*sys_iterate_files)(const struct SimSysIterator *iter);
	int (*sys_file_read)(const struct SimSysFile *file, char *buffer,
			int size, int offset);
	int (*sys_file_write)(const struct SimSysFile *file,
			const char *buffer, int size, int offset);
}
void sim_init(
	struct SimExported *exported, const struct SimImported *imported_host_ops,
	/* struct SimKernel *kernel // */
	const char* fmt, ...
)
{
	char command_line[COMMAND_LINE_SIZE] = {0};
	g_imported = imported;
	g_kernel = kernel;

//	exported->task_get_private = lkl_task_get_private;

	// or call the start function from DCE ?
	lkl_start_kernel(
			imported_host_ops,
			""
	);
}


/* for now rely on mutex since dce has everything available
 * _POSIX_SEMAPHORES
 * we could even move some of the sem
 * 
 * */
#undef _POSIX_SEMAPHORES

struct lkl_sem {
#ifdef _POSIX_SEMAPHORES
	sem_t sem;
#else
	pthread_mutex_t lock;
	int count;
	pthread_cond_t cond;
#endif /* _POSIX_SEMAPHORES */
};

/* lkl_thread_t
 * typedef unsigned long lkl_thread_t;
 * */
/* struct lkl_tls_key { */
/* 	pthread_key_t key; */
/* }; */

/* struct lkl_mutex { */
/* 	pthread_mutex_t mutex; */
/* }; */

/* #define WARN_UNLESS(exp) do {						\ */
/* 		if (exp < 0)						\ */
/* 			lkl_printf("%s: %s\n", #exp, strerror(errno));	\ */
/* 	} while (0) */

/* /1* struct SimTask { *1/ */
/* /1* 	struct task_struct kernel_task; *1/ */
/* /1* 	void *private; *1/ */
/* /1* }; *1/ */

/*   #define IMPORT_FROM_LKL(name */

/* /1* extern struct SimTask *sim_task_create(void *private, unsigned long pid); *1/ */
/* /1* sim_init *1/ */
/* /1* struct SimKernel * *1/ */
/* /1* see setup.c *1/ */
/* /1* void sim_init (m_exported, &imported, (struct SimKernel *)this) { *1/ */

/* /1*   } *1/ */

/* static int _warn_pthread(int ret, char *str_exp) */
/* { */
/* 	if (ret > 0) */
/* 		lkl_printf("%s: %s\n", str_exp, strerror(ret)); */

/* 	return ret; */
/* } */

/* /1* pthread_* functions use the reverse convention *1/ */
/* #define WARN_PTHREAD(exp) _warn_pthread(exp, #exp) */

/* static lkl_thread_t thread_create(void (*fn)(void *), void *arg) */
/* { */
/* 	pthread_t thread; */
/* 	if (WARN_PTHREAD(dce_pthread_create(&thread, NULL, (void* (*)(void *))fn, arg))) */
/* 		return 0; */
/* 	else */
/* 		return (lkl_thread_t) thread; */
/* } */

/* static void thread_detach(void) */
/* { */
/* 	WARN_PTHREAD(pthread_detach(pthread_self())); */
/* } */

/* static int thread_join(lkl_thread_t tid) */
/* { */
/* 	if (WARN_PTHREAD(dce_pthread_join((pthread_t)tid, NULL))) */
/* 		return -1; */
/* 	else */
/* 		return 0; */
/* } */

/* static void thread_exit(void) */
/* { */
/* 	dce_pthread_exit(NULL); */
/* } */

/* static int thread_equal(lkl_thread_t a, lkl_thread_t b) */
/* { */
/* 	return dce_pthread_equal(a, b); */
/* } */


/* /1* in dce utils.cc and dce-time.cc through functions like */ 
/*  * UtilsTimeToTimespec/ */
/*  * */
/*  * g_timeBase can set the initial time */
/*  *  *1/ */
/* static unsigned long long time_ns(void) */
/* { */
/* 	struct timespec ts; */

/* 	/1* DCE ignores CLOCK_MONOTONIC *1/ */
/* 	dce_clock_gettime(CLOCK_MONOTONIC, &ts); */

/* 	return 1e9*ts.tv_sec + ts.tv_nsec; */
/* } */


/* // TODO use host/node malloc */
/* static struct lkl_sem *sem_alloc(int count) */
/* { */
/* 	struct lkl_sem *sem; */

/* 	sem = malloc(sizeof(*sem)); */
/* 	if (!sem) */
/* 		return NULL; */

/* #ifdef _POSIX_SEMAPHORES */
/* 	if (sem_init(&sem->sem, SHARE_SEM, count) < 0) { */
/* 		lkl_printf("sem_init: %s\n", strerror(errno)); */
/* 		free(sem); */
/* 		return NULL; */
/* 	} */
/* #else */
/* 	pthread_mutex_init(&sem->lock, NULL); */
/* 	sem->count = count; */
/* 	WARN_PTHREAD(pthread_cond_init(&sem->cond, NULL)); */
/* #endif /1* _POSIX_SEMAPHORES *1/ */

/* 	return sem; */
/* } */

/* static void sem_free(struct lkl_sem *sem) */
/* { */
/* #ifdef _POSIX_SEMAPHORES */
/* 	WARN_UNLESS(sem_destroy(&sem->sem)); */
/* #else */
/* 	WARN_PTHREAD(pthread_cond_destroy(&sem->cond)); */
/* 	WARN_PTHREAD(pthread_mutex_destroy(&sem->lock)); */
/* #endif /1* _POSIX_SEMAPHORES *1/ */
/* 	free(sem); */
/* } */

/* static void sem_up(struct lkl_sem *sem) */
/* { */
/* #ifdef _POSIX_SEMAPHORES */
/* 	WARN_UNLESS(sem_post(&sem->sem)); */
/* #else */
/* 	WARN_PTHREAD(pthread_mutex_lock(&sem->lock)); */
/* 	sem->count++; */
/* 	if (sem->count > 0) */
/* 		WARN_PTHREAD(pthread_cond_signal(&sem->cond)); */
/* 	WARN_PTHREAD(pthread_mutex_unlock(&sem->lock)); */
/* #endif /1* _POSIX_SEMAPHORES *1/ */

/* } */

/* static void sem_down(struct lkl_sem *sem) */
/* { */
/* #ifdef _POSIX_SEMAPHORES */
/* 	int err; */

/* 	do { */
/* 		err = sem_wait(&sem->sem); */
/* 	} while (err < 0 && errno == EINTR); */
/* 	if (err < 0 && errno != EINTR) */
/* 		lkl_printf("sem_wait: %s\n", strerror(errno)); */
/* #else */
/* 	WARN_PTHREAD(pthread_mutex_lock(&sem->lock)); */
/* 	while (sem->count <= 0) */
/* 		WARN_PTHREAD(pthread_cond_wait(&sem->cond, &sem->lock)); */
/* 	sem->count--; */
/* 	WARN_PTHREAD(pthread_mutex_unlock(&sem->lock)); */
/* #endif /1* _POSIX_SEMAPHORES *1/ */
/* } */

/* static struct lkl_mutex *mutex_alloc(int recursive) */
/* { */
/* 	struct lkl_mutex *_mutex = malloc(sizeof(struct lkl_mutex)); */
/* 	pthread_mutex_t *mutex = NULL; */
/* 	pthread_mutexattr_t attr; */

/* 	if (!_mutex) */
/* 		return NULL; */

/* 	mutex = &_mutex->mutex; */
/* 	WARN_PTHREAD(dce_pthread_mutexattr_init(&attr)); */

/* 	/1* PTHREAD_MUTEX_ERRORCHECK is *very* useful for debugging, */
/* 	 * but has some overhead, so we provide an option to turn it */
/* 	 * off. *1/ */
/* #ifdef DEBUG */
/* 	if (!recursive) */
/* 		WARN_PTHREAD(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)); */
/* #endif /1* DEBUG *1/ */

/* 	if (recursive) */
/* 		WARN_PTHREAD(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)); */

/* 	WARN_PTHREAD(pthread_mutex_init(mutex, &attr)); */

/* 	return _mutex; */
/* } */

/* static void mutex_lock(struct lkl_mutex *mutex) */
/* { */
/* 	WARN_PTHREAD(pthread_mutex_lock(&mutex->mutex)); */
/* } */

/* static void mutex_unlock(struct lkl_mutex *_mutex) */
/* { */
/* 	pthread_mutex_t *mutex = &_mutex->mutex; */
/* 	WARN_PTHREAD(pthread_mutex_unlock(mutex)); */
/* } */

/* static void mutex_free(struct lkl_mutex *_mutex) */
/* { */
/* 	pthread_mutex_t *mutex = &_mutex->mutex; */
/* 	WARN_PTHREAD(pthread_mutex_destroy(mutex)); */
/* 	free(_mutex); */
/* } */


/* static struct lkl_tls_key* tls_alloc(void (*destructor)(void *)) */
/* { */
/* 	// NOP */
/* 	return NULL; */
/* } */

/* static void tls_free(struct lkl_tls_key *key) */
/* { */
/* 	// NOP */
/* } */

/* static int tls_set(struct lkl_tls_key *key, void *data) */
/* { */
/* 	// NOP */
/* 	return 0; */
/* } */

/* static void* tls_get(struct lkl_tls_key *key) */
/* { */
/* 	// NOP */
/* 	return NULL; */
/* } */

/* //static void* mem_alloc(unsigned long count) */
/* //{ */
/* //	// NOP */
/* //	return sim_malloc(count); */
/* //} */

/* //static void mem_free(void *addr) */
/* //{ */
/* //	// NOP */
/* //} */

/* /1* maybe we can do better with dce *1/ */
/* static void *timer_alloc(void (*fn)(void *), void *arg) */
/* { */
/* 	int err; */
/* 	timer_t timer; */
/* 	struct sigevent se =  { */
/* 		.sigev_notify = SIGEV_THREAD, */
/* 		.sigev_value = { */
/* 			.sival_ptr = arg, */
/* 		}, */
/* 		.sigev_notify_function = (void (*)(union sigval))fn, */
/* 	}; */

/* 	err = dce_timerfd_create(CLOCK_REALTIME, &se, &timer); */
/* 	if (err) */
/* 		return NULL; */

/* 	return (void *)(long)timer; */
/* } */

/* static int timer_set_oneshot(void *_timer, unsigned long ns) */
/* { */
/* 	timer_t timer = (timer_t)(long)_timer; */
/* 	struct itimerspec ts = { */
/* 		.it_value = { */
/* 			.tv_sec = ns / 1000000000, */
/* 			.tv_nsec = ns % 1000000000, */
/* 		}, */
/* 	}; */

/* 	return dce_timerfd_settime(timer, 0, &ts, NULL); */
/* } */

/* static void timer_free(void *_timer) */
/* { */
/* 	timer_t timer = (timer_t)(long)_timer; */

/* 	timer_delete(timer); */
/* } */

/* static void* lkl_ioremap(long addr, int size) */
/* { */
/* 	// NOP */
/* } */

/* static int lkl_iomem_access(const __volatile__ void *addr, void *val, int size, */
/* 		int write) */
/* { */
/* 	// NOP */
/* } */

/* static long gettid(void) */
/* { */
/* 	// returns a pid_t, */
/* 	return dce_gettid(); */
/* } */

/* /1* static void jmp_buf_set(struct lkl_jmp_buf *jmpb, void (*f)(void)) *1/ */
/* /1* { *1/ */
/* /1* 	// NOP *1/ */
/* /1* } *1/ */

/* /1* static void jmp_buf_longjmp(struct lkl_jmp_buf *jmpb, int val) *1/ */
/* /1* { *1/ */
/* /1* 	// NOP *1/ */
/* /1* } *1/ */
/* static lkl_thread_t thread_self(void) */
/* { */
/* 	return (lkl_thread_t)dce_pthread_self(); */
/* } */

/* static void print(const char *str, int len) */
/* { */
/* 	/1* int ret __attribute__((unused)); *1/ */
/* 	/1* ret = write(STDOUT_FILENO, str, len); *1/ */
/* 	dce_printf(str); */
/* } */


/* /1* look at posix host for some understanding *1/ */
/* struct lkl_host_operations lkl_host_ops = { */
/* 	.print = print, */
/* 	/1* for now a paste of dce_abort *1/ */
/* 	.panic = dce_panic, */

/* 	/1* most of these are available already in DCE */
/* 	 * we just use wrappers to fix the return type *1/ */
/* 	.thread_create = thread_create, */
/* 	.thread_detach = thread_detach, */
/* 	.thread_exit = thread_exit,  /1* returns void *1/ */
/* 	.thread_join = thread_join, */
/* 	.thread_self = thread_self, */
/* 	.thread_equal = thread_equal, */

/* 	/1* using posix version *1/ */
/* 	.sem_alloc = sem_alloc, */
/* 	.sem_free = sem_free, */
/* 	.sem_up = sem_up, */
/* 	.sem_down = sem_down, */

/* 	/1* posix inspired *1/ */
/* 	.mutex_alloc = mutex_alloc, */
/* 	.mutex_free = mutex_free, */
/* 	.mutex_lock = mutex_lock, */
/* 	.mutex_unlock = mutex_unlock, */

/* 	/1* todo *1/ */
/* 	.tls_alloc = tls_alloc, */
/* 	.tls_free = tls_free, */
/* 	.tls_set = tls_set, */
/* 	.tls_get = tls_get, */

/* 	.time = time_ns, */
/* 	.timer_alloc = timer_alloc, */
/* 	.timer_set_oneshot = timer_set_oneshot, */
/* 	.timer_free = timer_free, */

/* 	.mem_alloc = dce_malloc, */
/* 	.mem_free = dce_free, */

/* 	/1* TODO *1/ */
/* 	.ioremap = lkl_ioremap, */
/* 	.iomem_access = lkl_iomem_access, */
/* 	.virtio_devices = lkl_virtio_devs, */

/*   /1* @gettid - returns the host thread id of the caller, which need not *1/ */
/*   /1* be the same as the handle returned by thread_create *1/ */
/* 	.gettid = gettid, */

/* 	/1* pthread_fiber_manager does some things */
/* 	 * maybe try to reestablish the single codepath *1/ */
/* 	.jmp_buf_set = 0, */
/* 	.jmp_buf_longjmp = 0,  /1* jmp_buf_longjmp *1/ */
/* }; */
