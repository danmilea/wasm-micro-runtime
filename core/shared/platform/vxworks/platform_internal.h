/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _PLATFORM_INTERNAL_H
#define _PLATFORM_INTERNAL_H

#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sched.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BH_PLATFORM_VXWORKS
#define BH_PLATFORM_VXWORKS
#endif

#if !defined(O_DIRECTORY)
#define	O_DIRECTORY	0x00020000	/* Fail if not directory */
#endif
#if !defined(O_NOFOLLOW)
#define	O_NOFOLLOW	0x0100		/* don't follow symlinks */
#endif

#if !defined(AT_SYMLINK_FOLLOW)
#define	AT_SYMLINK_FOLLOW	0x0400	/* Follow symbolic link */
#endif

#if !defined(AT_SYMLINK_NOFOLLOW)
#define	AT_SYMLINK_NOFOLLOW	0x0200	/* Do not follow symbolic links */
#endif

#if !defined( AT_FDCWD)
#define AT_FDCWD (-100)
#endif

#if !defined(PATH_MAX)
#define PATH_MAX _POSIX_PATH_MAX
#endif

#if !defined(AT_REMOVEDIR)
#define AT_REMOVEDIR 0x200
#endif

#undef INET6

#ifndef S_ISSOCK
#define S_ISSOCK(mode) ((mode & S_IFMT) == S_IFSOCK) // Is file a socket?
#endif

#if !defined(IP_MULTICAST_LOOP)
#define IP_MULTICAST_LOOP   11
#endif

#if !defined(IP_ADD_MEMBERSHIP)
#define IP_ADD_MEMBERSHIP   12
#endif

#if !defined(IP_DROP_MEMBERSHIP)
#define IP_DROP_MEMBERSHIP  13
#endif

#if !defined(IP_TTL)
#define IP_TTL              4
#endif

#if !defined(IP_MULTICAST_TTL)
#define IP_MULTICAST_TTL    10
#endif

#if !defined(FIOGETNAME)
#define FIOGETNAME 18
#endif

#define os_alloca __builtin_alloca

/* Stack size of applet threads's native part.  */
#define BH_APPLET_PRESERVED_STACK_SIZE (32 * 1024)

/* Default thread priority */
#define BH_THREAD_DEFAULT_PRIORITY 0

typedef pthread_t korp_tid;
typedef pthread_mutex_t korp_mutex;
typedef pthread_cond_t korp_cond;
typedef pthread_t korp_thread;
typedef sem_t korp_sem;

#define OS_THREAD_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

#define os_thread_local_attribute __thread

#if WASM_DISABLE_HW_BOUND_CHECK == 0
#if defined(BUILD_TARGET_X86_64) || defined(BUILD_TARGET_AMD_64) \
    || defined(BUILD_TARGET_AARCH64)

#include <setjmp.h>

#define OS_ENABLE_HW_BOUND_CHECK

typedef jmp_buf korp_jmpbuf;

#define os_setjmp setjmp
#define os_longjmp longjmp
#define os_alloca __builtin_alloca

#define os_getpagesize getpagesize

typedef void (*os_signal_handler)(void *sig_addr);

int
os_thread_signal_init(os_signal_handler handler);

void
os_thread_signal_destroy();

bool
os_thread_signal_inited();

void
os_signal_unmask();

void
os_sigreturn();
#endif /* end of BUILD_TARGET_X86_64/AMD_64/AARCH64 */
#endif /* end of WASM_DISABLE_HW_BOUND_CHECK */

struct ip_mreq {
    struct  in_addr imr_multiaddr;  /* IP multicast address of group */
    struct  in_addr imr_interface;  /* local IP address of interface */
};

ssize_t pread(int fd, void * buf, size_t size, off_t offset);
ssize_t pwrite(int fd, const void * buf, size_t size, off_t offset);
int openat(int fd, const char *path, int oflags, ...);
ssize_t readlinkat(int dirfd, const char *restrict pathname,
                        char *restrict buf, size_t bufsiz);
int mkdirat(int dirfd, const char *pathname, mode_t mode);
int linkat(int olddirfd, const char *oldpath, int newdirfd,
                const char *newpath, int flags);
int symlinkat(const char *target, int newdirfd, const char *linkpath);
int fstatat(int fd, const char *restrict path,
           struct stat *restrict buf, int flag);
DIR *fdopendir(int fd);
void seekdir(DIR *dirp, long loc);
long telldir(DIR *dirp);
int renameat(int olddirfd, const char *oldpath,
             int newdirfd, const char *newpath);
int unlinkat(int dirfd, const char *pathname, int flags);
int utimensat(int fd, const char *path, const struct timespec times[2],
           int flag);
int getentropy(void *buffer, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* end of _PLATFORM_INTERNAL_H */
