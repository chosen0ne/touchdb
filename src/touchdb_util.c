/**
 * touchdb_util.c
 *
 * @author chosen0ne
 * @date 2012-06-20
 *
 * Copyright (C) by chosen0ne
 */

#include <pthread.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "touchdb_common.h"
#include "touchdb_util.h"

#define MAXLINE 100

static void
err_doit(touchdb_log_level level, const char *fmt, va_list ap){
	int 	errno_save, n;
	char 	buf[MAXLINE + 1];

	errno_save = errno;
#ifdef	HAVE_VSNPRINTF
	vsnprintf(buf, MAXLINE, fmt, ap);	/* safe */
#else
	vsprintf(buf, fmt, ap);				/* not safe */
#endif
	strcat(buf, ", msg %s");
	touchdb_log(level, buf, strerror(errno));
	return;
}

void
err_sys(const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	err_doit(TOUCHDB_LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

/**
 * ends with NULL
 */
void touchdb_strcat(char *dist, ...){
	va_list 	ap;
	const char	*s;

	printf(":sfasf");
	va_start(ap, dist);
	while(1){
		s = va_arg(ap, char *);
		if(s == NULL)
			break;
		strcat(dist, s);
	}
	va_end(ap);
}

/**
 * ends with NULL
 */
void touchdb_strcpy(char *dist, ...){
	va_list		ap;
	const char	*s;

	va_start(ap, dist);
	while(1){
		s = va_arg(ap, char *);
		if(s == NULL)
			break;
		strcpy(dist, s);
		dist += strlen(s);
	}
	va_end(ap);
}

void
Pthread_mutexattr_init(pthread_mutexattr_t *mxattr, const char *msg){
	int 	n;

	if((n = pthread_mutexattr_init(mxattr)) == 0)
		return;
	errno = n;
	err_sys("pthread_mutexattr_init error", msg);
}

void
Pthread_mutex_init(pthread_mutex_t *mx, pthread_mutexattr_t *mxattr, const char *msg){
	int 	n;

	if((n = pthread_mutex_init(mx, mxattr)) == 0)
		return;
	errno = n;
	err_sys("pthread_mutex_init error, msg: %s", msg);
}


void
Pthread_mutexattr_setpshared(pthread_mutexattr_t *mxattr, int val, const char *msg){
	int 	n;
	if((n = pthread_mutexattr_setpshared(mxattr, val)) == 0)
		return;
	errno = n;
	err_sys("pthread_mutexattr_setpshared error, msg: %s", msg);
}

void
Pthread_mutex_lock(pthread_mutex_t *mx, const char *msg){
	int		n;

	if((n = pthread_mutex_lock(mx)) == 0)
		return;
	errno = n;
	err_sys("pthread_mutex_lock error, msg: %s", msg);
}
void
Pthread_mutex_unlock(pthread_mutex_t *mx, const char *msg){
	int		n;

	if((n = pthread_mutex_unlock(mx)) == 0)
		return;
	errno = n;
	err_sys("pthread_mutex_unlock, msg: %s", msg);
}

void
Pthread_rwlockattr_init(pthread_rwlockattr_t *lckattr, const char *msg){
	int		n;
	if((n = pthread_rwlockattr_init(lckattr)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlockattr_init error, msg: %s", msg);
}
void
Pthread_rwlock_init(pthread_rwlock_t *lck, pthread_rwlockattr_t *lckattr, const char *msg){
	int 	n;
	if((n = pthread_rwlock_init(lck, lckattr)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_init error, msg: %s", msg);
}
void
Pthread_rwlockattr_setpshared(pthread_rwlockattr_t *lckattr, int val, const char *msg){
	int		n;
	if((n = pthread_rwlockattr_setpshared(lckattr, val)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlockattr_setpshared error, msg: %s", msg);
}

void
Pthread_rwlock_rdlock(pthread_rwlock_t *lck, const char *msg){
	int 	n;
	if((n = pthread_rwlock_rdlock(lck)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_rdlock error, msg: %s", msg);
}

void
Pthread_rwlock_wrlock(pthread_rwlock_t *lck, const char *msg){
	int 	n;
	if((n = pthread_rwlock_wrlock(lck)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_wrlock error, msg: %s", msg);
}

void
Pthread_rwlock_unlock(pthread_rwlock_t *lck, const char *msg){
	int 	n;
	if((n = pthread_rwlock_unlock(lck)) == 0)
		return;
	errno = n;
	err_sys("pthread_rwlock_unlock error, msg: %s", msg);
}
