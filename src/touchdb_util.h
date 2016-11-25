/**
 * touchdb_util.h
 *
 * @author chosen0ne
 * @date 2012-06-20
 *
 * Copyright (C) by chosen0ne
 */

#ifndef TOUCHDB_UTIL_H
#define TOUCHDB_UTIL_H

void err_sys(const char *fmt, ...);
void err_msg(const char *fmt, ...);

void Pthread_mutex_init(pthread_mutex_t *mx, pthread_mutexattr_t *mxattr, const char *msg);
void Pthread_mutexattr_init(pthread_mutexattr_t *mxattr, const char *msg);
void Pthread_mutexattr_setpshared(pthread_mutexattr_t *mxattr, int val, const char *msg);
void Pthread_mutex_lock(pthread_mutex_t *mx, const char *msg);
void Pthread_mutex_unlock(pthread_mutex_t *mx, const char *msg);

void Pthread_rwlockattr_init(pthread_rwlockattr_t *lckattr, const char *msg);
void Pthread_rwlock_init(pthread_rwlock_t *lck, pthread_rwlockattr_t *lckattr, const char *msg);
void Pthread_rwlockattr_setpshared(pthread_rwlockattr_t *lckattr, int val, const char *msg);
void Pthread_rwlock_rdlock(pthread_rwlock_t *lck, const char *msg);
void Pthread_rwlock_wrlock(pthread_rwlock_t *lck, const char *msg);
void Pthread_rwlock_unlock(pthread_rwlock_t *lck, const char *msg);

void touchdb_strcat(char *dist, ...);
void touchdb_strcpy(char *dist, ...);
#endif
