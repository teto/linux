/*
 * Userspace system call emulation via LD_PRELOAD libc system functions
 * redirecting to LKL.
 *
 * Copyright (c) 2015 Hajime Tazaki
 *
 * Author: Hajime Tazaki <tazaki@sfc.wide.ad.jp>
 *
 * Note: some of the code is picked from rumpkernel, written by Antti Kantee.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#define __USE_GNU
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_tun.h>
#undef st_atime
#undef st_mtime
#undef st_ctime
#include <lkl.h>
#include <lkl_host.h>

static long set_errno(long err)
{
	if (err >= 0)
		return err;

	switch (err) {
	case -LKL_EPERM:
		errno = EPERM;
		break;
	case -LKL_ENOENT:
		errno = ENOENT;
		break;
	case -LKL_ESRCH:
		errno = ESRCH;
		break;
	case -LKL_EINTR:
		errno = EINTR;
		break;
	case -LKL_EIO:
		errno = EIO;
		break;
	case -LKL_ENXIO:
		errno = ENXIO;
		break;
	case -LKL_E2BIG:
		errno = E2BIG;
		break;
	case -LKL_ENOEXEC:
		errno = ENOEXEC;
		break;
	case -LKL_EBADF:
		errno = EBADF;
		break;
	case -LKL_ECHILD:
		errno = ECHILD;
		break;
	case -LKL_EAGAIN:
		errno = EAGAIN;
		break;
	case -LKL_ENOMEM:
		errno = ENOMEM;
		break;
	case -LKL_EACCES:
		errno = EACCES;
		break;
	case -LKL_EFAULT:
		errno = EFAULT;
		break;
	case -LKL_ENOTBLK:
		errno = ENOTBLK;
		break;
	case -LKL_EBUSY:
		errno = EBUSY;
		break;
	case -LKL_EEXIST:
		errno = EEXIST;
		break;
	case -LKL_EXDEV:
		errno = EXDEV;
		break;
	case -LKL_ENODEV:
		errno = ENODEV;
		break;
	case -LKL_ENOTDIR:
		errno = ENOTDIR;
		break;
	case -LKL_EISDIR:
		errno = EISDIR;
		break;
	case -LKL_EINVAL:
		errno = EINVAL;
		break;
	case -LKL_ENFILE:
		errno = ENFILE;
		break;
	case -LKL_EMFILE:
		errno = EMFILE;
		break;
	case -LKL_ENOTTY:
		errno = ENOTTY;
		break;
	case -LKL_ETXTBSY:
		errno = ETXTBSY;
		break;
	case -LKL_EFBIG:
		errno = EFBIG;
		break;
	case -LKL_ENOSPC:
		errno = ENOSPC;
		break;
	case -LKL_ESPIPE:
		errno = ESPIPE;
		break;
	case -LKL_EROFS:
		errno = EROFS;
		break;
	case -LKL_EMLINK:
		errno = EMLINK;
		break;
	case -LKL_EPIPE:
		errno = EPIPE;
		break;
	case -LKL_EDOM:
		errno = EDOM;
		break;
	case -LKL_ERANGE:
		errno = ERANGE;
		break;
	case -LKL_EDEADLK:
		errno = EDEADLK;
		break;
	case -LKL_ENAMETOOLONG:
		errno = ENAMETOOLONG;
		break;
	case -LKL_ENOLCK:
		errno = ENOLCK;
		break;
	case -LKL_ENOSYS:
		errno = ENOSYS;
		break;
	case -LKL_ENOTEMPTY:
		errno = ENOTEMPTY;
		break;
	case -LKL_ELOOP:
		errno = ELOOP;
		break;
	case -LKL_ENOMSG:
		errno = ENOMSG;
		break;
	case -LKL_EIDRM:
		errno = EIDRM;
		break;
	case -LKL_ECHRNG:
		errno = ECHRNG;
		break;
	case -LKL_EL2NSYNC:
		errno = EL2NSYNC;
		break;
	case -LKL_EL3HLT:
		errno = EL3HLT;
		break;
	case -LKL_EL3RST:
		errno = EL3RST;
		break;
	case -LKL_ELNRNG:
		errno = ELNRNG;
		break;
	case -LKL_EUNATCH:
		errno = EUNATCH;
		break;
	case -LKL_ENOCSI:
		errno = ENOCSI;
		break;
	case -LKL_EL2HLT:
		errno = EL2HLT;
		break;
	case -LKL_EBADE:
		errno = EBADE;
		break;
	case -LKL_EBADR:
		errno = EBADR;
		break;
	case -LKL_EXFULL:
		errno = EXFULL;
		break;
	case -LKL_ENOANO:
		errno = ENOANO;
		break;
	case -LKL_EBADRQC:
		errno = EBADRQC;
		break;
	case -LKL_EBADSLT:
		errno = EBADSLT;
		break;
	case -LKL_EBFONT:
		errno = EBFONT;
		break;
	case -LKL_ENOSTR:
		errno = ENOSTR;
		break;
	case -LKL_ENODATA:
		errno = ENODATA;
		break;
	case -LKL_ETIME:
		errno = ETIME;
		break;
	case -LKL_ENOSR:
		errno = ENOSR;
		break;
	case -LKL_ENONET:
		errno = ENONET;
		break;
	case -LKL_ENOPKG:
		errno = ENOPKG;
		break;
	case -LKL_EREMOTE:
		errno = EREMOTE;
		break;
	case -LKL_ENOLINK:
		errno = ENOLINK;
		break;
	case -LKL_EADV:
		errno = EADV;
		break;
	case -LKL_ESRMNT:
		errno = ESRMNT;
		break;
	case -LKL_ECOMM:
		errno = ECOMM;
		break;
	case -LKL_EPROTO:
		errno = EPROTO;
		break;
	case -LKL_EMULTIHOP:
		errno = EMULTIHOP;
		break;
	case -LKL_EDOTDOT:
		errno = EDOTDOT;
		break;
	case -LKL_EBADMSG:
		errno = EBADMSG;
		break;
	case -LKL_EOVERFLOW:
		errno = EOVERFLOW;
		break;
	case -LKL_ENOTUNIQ:
		errno = ENOTUNIQ;
		break;
	case -LKL_EBADFD:
		errno = EBADFD;
		break;
	case -LKL_EREMCHG:
		errno = EREMCHG;
		break;
	case -LKL_ELIBACC:
		errno = ELIBACC;
		break;
	case -LKL_ELIBBAD:
		errno = ELIBBAD;
		break;
	case -LKL_ELIBSCN:
		errno = ELIBSCN;
		break;
	case -LKL_ELIBMAX:
		errno = ELIBMAX;
		break;
	case -LKL_ELIBEXEC:
		errno = ELIBEXEC;
		break;
	case -LKL_EILSEQ:
		errno = EILSEQ;
		break;
	case -LKL_ERESTART:
		errno = ERESTART;
		break;
	case -LKL_ESTRPIPE:
		errno = ESTRPIPE;
		break;
	case -LKL_EUSERS:
		errno = EUSERS;
		break;
	case -LKL_ENOTSOCK:
		errno = ENOTSOCK;
		break;
	case -LKL_EDESTADDRREQ:
		errno = EDESTADDRREQ;
		break;
	case -LKL_EMSGSIZE:
		errno = EMSGSIZE;
		break;
	case -LKL_EPROTOTYPE:
		errno = EPROTOTYPE;
		break;
	case -LKL_ENOPROTOOPT:
		errno = ENOPROTOOPT;
		break;
	case -LKL_EPROTONOSUPPORT:
		errno = EPROTONOSUPPORT;
		break;
	case -LKL_ESOCKTNOSUPPORT:
		errno = ESOCKTNOSUPPORT;
		break;
	case -LKL_EOPNOTSUPP:
		errno = EOPNOTSUPP;
		break;
	case -LKL_EPFNOSUPPORT:
		errno = EPFNOSUPPORT;
		break;
	case -LKL_EAFNOSUPPORT:
		errno = EAFNOSUPPORT;
		break;
	case -LKL_EADDRINUSE:
		errno = EADDRINUSE;
		break;
	case -LKL_EADDRNOTAVAIL:
		errno = EADDRNOTAVAIL;
		break;
	case -LKL_ENETDOWN:
		errno = ENETDOWN;
		break;
	case -LKL_ENETUNREACH:
		errno = ENETUNREACH;
		break;
	case -LKL_ENETRESET:
		errno = ENETRESET;
		break;
	case -LKL_ECONNABORTED:
		errno = ECONNABORTED;
		break;
	case -LKL_ECONNRESET:
		errno = ECONNRESET;
		break;
	case -LKL_ENOBUFS:
		errno = ENOBUFS;
		break;
	case -LKL_EISCONN:
		errno = EISCONN;
		break;
	case -LKL_ENOTCONN:
		errno = ENOTCONN;
		break;
	case -LKL_ESHUTDOWN:
		errno = ESHUTDOWN;
		break;
	case -LKL_ETOOMANYREFS:
		errno = ETOOMANYREFS;
		break;
	case -LKL_ETIMEDOUT:
		errno = ETIMEDOUT;
		break;
	case -LKL_ECONNREFUSED:
		errno = ECONNREFUSED;
		break;
	case -LKL_EHOSTDOWN:
		errno = EHOSTDOWN;
		break;
	case -LKL_EHOSTUNREACH:
		errno = EHOSTUNREACH;
		break;
	case -LKL_EALREADY:
		errno = EALREADY;
		break;
	case -LKL_EINPROGRESS:
		errno = EINPROGRESS;
		break;
	case -LKL_ESTALE:
		errno = ESTALE;
		break;
	case -LKL_EUCLEAN:
		errno = EUCLEAN;
		break;
	case -LKL_ENOTNAM:
		errno = ENOTNAM;
		break;
	case -LKL_ENAVAIL:
		errno = ENAVAIL;
		break;
	case -LKL_EISNAM:
		errno = EISNAM;
		break;
	case -LKL_EREMOTEIO:
		errno = EREMOTEIO;
		break;
	case -LKL_EDQUOT:
		errno = EDQUOT;
		break;
	case -LKL_ENOMEDIUM:
		errno = ENOMEDIUM;
		break;
	case -LKL_EMEDIUMTYPE:
		errno = EMEDIUMTYPE;
		break;
	case -LKL_ECANCELED:
		errno = ECANCELED;
		break;
	case -LKL_ENOKEY:
		errno = ENOKEY;
		break;
	case -LKL_EKEYEXPIRED:
		errno = EKEYEXPIRED;
		break;
	case -LKL_EKEYREVOKED:
		errno = EKEYREVOKED;
		break;
	case -LKL_EKEYREJECTED:
		errno = EKEYREJECTED;
		break;
	case -LKL_EOWNERDEAD:
		errno = EOWNERDEAD;
		break;
	case -LKL_ENOTRECOVERABLE:
		errno = ENOTRECOVERABLE;
		break;
	case -LKL_ERFKILL:
		errno = ERFKILL;
		break;
	case -LKL_EHWPOISON:
		errno = EHWPOISON;
		break;
	}

	return -1;
}

#define LKL_FD_OFFSET (FD_SETSIZE/2)

static int is_lklfd(int fd)
{
	if (fd < LKL_FD_OFFSET)
		return 0;

	return 1;
}

static void *resolve_sym(const char *sym)
{
	void *resolv;

	resolv = dlsym(RTLD_NEXT, sym);
	if (!resolv) {
		fprintf(stderr, "dlsym fail %s (%s)\n", sym, dlerror());
		assert(0);
	}
	return resolv;
}

typedef long (*host_call)(long p1, long p2, long p3, long p4, long p5, long p6);

static host_call host_calls[__lkl__NR_syscalls];

#define HOOK_FD_CALL(name)						\
	static void __attribute__((constructor(101)))			\
	init_host_##name(void)						\
	{								\
		host_calls[__lkl__NR_##name] = resolve_sym(#name);	\
	}								\
									\
	long name##_hook(long p1, long p2, long p3, long p4, long p5,	\
			 long p6)					\
	{								\
		long p[6] = {p1, p2, p3, p4, p5, p6 };			\
		long ret;						\
									\
		if (!is_lklfd(p1))					\
			return host_calls[__lkl__NR_##name](p1, p2, p3,	\
							    p4, p5, p6); \
									\
		ret = lkl_syscall(__lkl__NR_##name, p);			\
		return set_errno(ret);					\
	}								\
	asm(".global " #name);						\
	asm(".set " #name "," #name "_hook");				\

#define HOST_CALL(name)							\
	static long (*host_##name)();					\
	static void __attribute__((constructor(101)))			\
	init2_host_##name(void)						\
	{								\
		host_##name = resolve_sym(#name);			\
	}

#define HOOK_CALL(name)							\
	long name##_hook(long p1, long p2, long p3, long p4, long p5,	\
			 long p6)					\
	{								\
		long p[6] = {p1, p2, p3, p4, p5, p6};			\
									\
		return set_errno(lkl_syscall(__lkl__NR_##name, p));	\
	}								\
	asm(".global " #name);						\
	asm(".set " #name "," #name "_hook");				\


static int lkl_call(int nr, int args, ...)
{
	long params[6];
	va_list vl;
	int i;

	va_start(vl, args);
	for (i = 0; i < args; i++)
		params[i] = va_arg(vl, long);
	va_end(vl);

	return set_errno(lkl_syscall(nr, params));
}

HOOK_FD_CALL(close)
HOOK_FD_CALL(recvmsg)
HOOK_FD_CALL(sendmsg)
HOOK_FD_CALL(sendmmsg)
HOOK_FD_CALL(getsockname)
HOOK_FD_CALL(getpeername)
HOOK_FD_CALL(bind)
HOOK_FD_CALL(connect)
HOOK_FD_CALL(listen)
HOOK_FD_CALL(shutdown)
HOOK_FD_CALL(accept)
HOOK_FD_CALL(write)
HOOK_FD_CALL(writev)
HOOK_FD_CALL(sendto)
HOOK_FD_CALL(send)
HOOK_FD_CALL(read)
HOOK_FD_CALL(recvfrom)
HOOK_FD_CALL(recv)
HOOK_FD_CALL(epoll_wait)

static int soname_xlate(int soname)
{
	switch (soname) {
	case SO_DEBUG:
		return LKL_SO_DEBUG;
	case SO_REUSEADDR:
		return LKL_SO_REUSEADDR;
	case SO_TYPE:
		return LKL_SO_TYPE;
	case SO_ERROR:
		return LKL_SO_ERROR;
	case SO_DONTROUTE:
		return LKL_SO_DONTROUTE;
	case SO_BROADCAST:
		return LKL_SO_BROADCAST;
	case SO_SNDBUF:
		return LKL_SO_SNDBUF;
	case SO_RCVBUF:
		return LKL_SO_RCVBUF;
	case SO_SNDBUFFORCE:
		return LKL_SO_SNDBUFFORCE;
	case SO_RCVBUFFORCE:
		return LKL_SO_RCVBUFFORCE;
	case SO_KEEPALIVE:
		return LKL_SO_KEEPALIVE;
	case SO_OOBINLINE:
		return LKL_SO_OOBINLINE;
	case SO_NO_CHECK:
		return LKL_SO_NO_CHECK;
	case SO_PRIORITY:
		return LKL_SO_PRIORITY;
	case SO_LINGER:
		return LKL_SO_LINGER;
	case SO_BSDCOMPAT:
		return LKL_SO_BSDCOMPAT;
#ifdef SO_REUSEPORT
	case SO_REUSEPORT:
		return LKL_SO_REUSEPORT;
#endif
	case SO_PASSCRED:
		return LKL_SO_PASSCRED;
	case SO_PEERCRED:
		return LKL_SO_PEERCRED;
	case SO_RCVLOWAT:
		return LKL_SO_RCVLOWAT;
	case SO_SNDLOWAT:
		return LKL_SO_SNDLOWAT;
	case SO_RCVTIMEO:
		return LKL_SO_RCVTIMEO;
	case SO_SNDTIMEO:
		return LKL_SO_SNDTIMEO;
	case SO_SECURITY_AUTHENTICATION:
		return LKL_SO_SECURITY_AUTHENTICATION;
	case SO_SECURITY_ENCRYPTION_TRANSPORT:
		return LKL_SO_SECURITY_ENCRYPTION_TRANSPORT;
	case SO_SECURITY_ENCRYPTION_NETWORK:
		return LKL_SO_SECURITY_ENCRYPTION_NETWORK;
	case SO_BINDTODEVICE:
		return LKL_SO_BINDTODEVICE;
	case SO_ATTACH_FILTER:
		return LKL_SO_ATTACH_FILTER;
	case SO_DETACH_FILTER:
		return LKL_SO_DETACH_FILTER;
	case SO_PEERNAME:
		return LKL_SO_PEERNAME;
	case SO_TIMESTAMP:
		return LKL_SO_TIMESTAMP;
	case SO_ACCEPTCONN:
		return LKL_SO_ACCEPTCONN;
	case SO_PEERSEC:
		return LKL_SO_PEERSEC;
	case SO_PASSSEC:
		return LKL_SO_PASSSEC;
	case SO_TIMESTAMPNS:
		return LKL_SO_TIMESTAMPNS;
	case SO_MARK:
		return LKL_SO_MARK;
	case SO_TIMESTAMPING :
		return LKL_SO_TIMESTAMPING;
	case SO_PROTOCOL:
		return LKL_SO_PROTOCOL;
	case SO_DOMAIN:
		return LKL_SO_DOMAIN;
	case SO_RXQ_OVFL:
		return LKL_SO_RXQ_OVFL;
#ifdef SO_WIFI_STATUS
	case SO_WIFI_STATUS:
		return LKL_SO_WIFI_STATUS;
#endif
#ifdef SO_PEEK_OFF
	case SO_PEEK_OFF:
		return LKL_SO_PEEK_OFF;
#endif
#ifdef SO_NOFCS
	case SO_NOFCS:
		return LKL_SO_NOFCS;
#endif
#ifdef SO_LOCK_FILTER
	case SO_LOCK_FILTER:
		return LKL_SO_LOCK_FILTER;
#endif
#ifdef SO_SELECT_ERR_QUEUE
	case SO_SELECT_ERR_QUEUE:
		return LKL_SO_SELECT_ERR_QUEUE;
#endif
#ifdef SO_BUSY_POLL
	case SO_BUSY_POLL:
		return LKL_SO_BUSY_POLL;
#endif
#ifdef SO_MAX_PACING_RATE
	case SO_MAX_PACING_RATE:
		return LKL_SO_MAX_PACING_RATE;
#endif
	}

	return soname;
}

static int solevel_xlate(int solevel)
{
	switch (solevel) {
	case SOL_SOCKET:
		return LKL_SOL_SOCKET;
	}

	return solevel;
}

HOST_CALL(setsockopt);
int setsockopt(int fd, int level, int optname, const void *optval,
	       socklen_t optlen)
{
	if (!is_lklfd(fd))
		return host_setsockopt(fd, level, optname, optval, optlen);
	return set_errno(lkl_sys_setsockopt(fd, solevel_xlate(level),
					    soname_xlate(optname),
					    (void*)optval, optlen));
}

HOST_CALL(getsockopt);
int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
	if (!is_lklfd(fd))
		return host_setsockopt(fd, level, optname, optval, optlen);
	return set_errno(lkl_sys_getsockopt(fd, solevel_xlate(level),
					    soname_xlate(optname), optval,
					    (int*)optlen));
}

HOST_CALL(socket);
int socket(int domain, int type, int protocol)
{
	int ret;

	if (domain == AF_UNIX)
		return host_socket(domain, type, protocol);

	ret = lkl_sys_socket(domain, type, protocol);
	return set_errno(ret);
}

static unsigned long ioctl_req_xlate(unsigned long req)
{
	switch (req) {
	case FIOSETOWN:
		return LKL_FIOSETOWN;
	case SIOCSPGRP:
		return LKL_SIOCSPGRP;
	case FIOGETOWN:
		return LKL_FIOGETOWN;
	case SIOCGPGRP:
		return LKL_SIOCGPGRP;
	case SIOCATMARK:
		return LKL_SIOCATMARK;
	case SIOCGSTAMP:
		return LKL_SIOCGSTAMP;
	case SIOCGSTAMPNS:
		return LKL_SIOCGSTAMPNS;
	}

	/* TODO: asm/termios.h translations */

	return req;
}

HOST_CALL(ioctl);
int ioctl(int fd, unsigned long req, ...)
{
	va_list vl;
	long arg;

	va_start(vl, req);
	arg = va_arg(vl, long);
	va_end(vl);

	if (!is_lklfd(fd))
		return host_ioctl(fd, req, arg);
	return set_errno(lkl_sys_ioctl(fd, ioctl_req_xlate(req), arg));
}

static int fcntl_cmd_xlate(int cmd)
{
	switch (cmd) {
	case F_DUPFD:
		return LKL_F_DUPFD;
	case F_GETFD:
		return LKL_F_GETFD;
	case F_SETFD:
		return LKL_F_SETFD;
	case F_GETFL:
		return LKL_F_GETFL;
	case F_SETFL:
		return LKL_F_SETFL;
	case F_GETLK:
		return LKL_F_GETLK;
	case F_SETLK:
		return LKL_F_SETLK;
	case F_SETLKW:
		return LKL_F_SETLKW;
	case F_SETOWN:
		return LKL_F_SETOWN;
	case F_GETOWN:
		return LKL_F_GETOWN;
	case F_SETSIG:
		return LKL_F_SETSIG;
	case F_GETSIG:
		return LKL_F_GETSIG;
#ifndef LKL_CONFIG_64BIT
	case F_GETLK64:
		return LKL_F_GETLK64;
	case F_SETLK64:
		return LKL_F_SETLK64;
	case F_SETLKW64:
		return LKL_F_SETLKW64;
#endif
	case F_SETOWN_EX:
		return LKL_F_SETOWN_EX;
	case F_GETOWN_EX:
		return LKL_F_GETOWN_EX;
	}

	return cmd;
}

HOST_CALL(fcntl);
int fcntl(int fd, int cmd, ...)
{
	va_list vl;
	long arg;

	va_start(vl, cmd);
	arg = va_arg(vl, long);
	va_end(vl);

	if (!is_lklfd(fd))
		return host_fcntl(fd, cmd, arg);
	return set_errno(lkl_sys_fcntl(fd, fcntl_cmd_xlate(cmd), arg));
}

HOOK_CALL(pipe);

HOST_CALL(poll);
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	unsigned int i, lklfds = 0, hostfds = 0;

	for (i = 0; i < nfds; i++) {
		if (is_lklfd(fds[i].fd))
			lklfds = 1;
		else
			hostfds = 1;
	}

	/* FIXME: need to handle mixed case of hostfd and lklfd. */
	if (lklfds && hostfds)
		return set_errno(-LKL_EOPNOTSUPP);

	if (hostfds)
		return host_poll(fds, nfds, timeout);

	return set_errno(lkl_call(__lkl__NR_poll, 3, fds, nfds, timeout));
}

int __poll(struct pollfd *, nfds_t, int) __attribute__((alias("poll")));

HOST_CALL(select);
int select(int nfds, fd_set *r, fd_set *w, fd_set *e, struct timeval *t)
{
	int fd, hostfds = 0, lklfds = 0;

	for (fd = 0; fd < nfds; fd++) {
		if (r != 0 && FD_ISSET(fd, r)) {
			if (is_lklfd(fd))
				lklfds = 1;
			else
				hostfds = 1;
		}
		if (w != 0 && FD_ISSET(fd, w)) {
			if (is_lklfd(fd))
				lklfds = 1;
			else
				hostfds = 1;
		}
		if (e != 0 && FD_ISSET(fd, e)) {
			if (is_lklfd(fd))
				lklfds = 1;
			else
				hostfds = 1;
		}
	}

	/* FIXME: handle mixed case of hostfd and lklfd */
	if (lklfds && hostfds)
		return set_errno(-EOPNOTSUPP);

	if (hostfds)
		return host_select(nfds, r, w, e, t);

	return set_errno(lkl_call(__lkl__NR_select, 5, nfds, r, w, e, t));
}

HOOK_CALL(epoll_create)

HOST_CALL(epoll_ctl);
int epoll_ctl(int epollfd, int op, int fd, struct epoll_event *event)
{
	if (is_lklfd(epollfd) != is_lklfd(fd))
		return set_errno(-LKL_EOPNOTSUPP);

	if (!is_lklfd(epollfd))
		return host_epoll_ctl(epollfd, op, fd, event);

	return lkl_call(__lkl__NR_epoll_ctl, 3, op, fd, event);
}

static void __attribute__((constructor(102))) init(void)
{
	long dev_null;
	int ret, i, nd_id = -1, nd_ifindex = -1;
	char *tap = getenv("LKL_PRELOAD_NET_TAP");
	char *ip = getenv("LKL_PRELOAD_NET_IP");
	char *netmask_len = getenv("LKL_PRELOAD_NET_NETMASK_LEN");
	char *gateway = getenv("LKL_PRELOAD_NET_GATEWAY");
	char *debug = getenv("LKL_PRELOAD_DEBUG");

	if (tap) {
		struct ifreq ifr = {
			.ifr_flags = IFF_TAP | IFF_NO_PI,
		};
		union lkl_netdev nd;

		strncpy(ifr.ifr_name, tap, IFNAMSIZ);

		nd.fd = open("/dev/net/tun", O_RDWR|O_NONBLOCK);
		if (nd.fd < 0) {
			fprintf(stderr, "failed to open tap: %s\n", strerror(errno));
			goto no_tap;
		}

		ret = ioctl(nd.fd, TUNSETIFF, &ifr);
		if (ret < 0) {
			fprintf(stderr, "failed to attach to %s: %s\n",
				ifr.ifr_name, strerror(errno));
			goto no_tap;
		}

		ret = lkl_netdev_add(nd, NULL);
		if (ret < 0) {
			fprintf(stderr, "failed to add netdev: %s\n",
				lkl_strerror(ret));
			goto no_tap;
		}

		nd_id = ret;
	}

no_tap:

	if (!debug)
		lkl_host_ops.print = NULL;

	ret = lkl_start_kernel(&lkl_host_ops, 64 * 1024 * 1024, "");
	if (ret) {
		fprintf(stderr, "can't start kernel: %s\n", lkl_strerror(ret));
		return;
	}

	/* fillup FDs up to LKL_FD_OFFSET */
	ret = lkl_sys_mknod("/dev_null", LKL_S_IFCHR | 0600, LKL_MKDEV(1, 3));
	dev_null = lkl_sys_open("/dev_null", LKL_O_RDONLY, 0);
	if (dev_null < 0) {
		fprintf(stderr, "failed to open /dev/null: %s\n", lkl_strerror(dev_null));
		return;
	}

	for (i = 1; i < LKL_FD_OFFSET; i++)
		lkl_sys_dup(dev_null);

	lkl_if_up(1);

	if (nd_id >= 0) {
		nd_ifindex = lkl_netdev_get_ifindex(nd_id);
		if (nd_ifindex > 0)
			lkl_if_up(nd_ifindex);
		else
			fprintf(stderr, "failed to get ifindex for netdev id %d: %s\n",
				nd_id, lkl_strerror(nd_ifindex));
	}

	if (nd_ifindex >= 0 && ip && netmask_len) {
		unsigned int addr = inet_addr(ip);
		int nmlen = atoi(netmask_len);

		if (addr != INADDR_NONE && nmlen > 0 && nmlen < 32) {
			ret = lkl_if_set_ipv4(nd_ifindex, addr, nmlen);
			if (ret < 0)
				fprintf(stderr, "failed to set IPv4 address: %s\n",
					lkl_strerror(ret));
		}
	}

	if (nd_ifindex >= 0 && gateway) {
		unsigned int addr = inet_addr(gateway);

		if (addr != INADDR_NONE) {
			ret = lkl_set_ipv4_gateway(addr);
			if (ret< 0)
				fprintf(stderr, "failed to set IPv4 gateway: %s\n",
					lkl_strerror(ret));
		}
	}
}

static void __attribute__((destructor)) fini(void)
{
	int i;

	for (i = 0; i < LKL_FD_OFFSET; i++)
		lkl_sys_close(i);

	lkl_sys_halt();
}
