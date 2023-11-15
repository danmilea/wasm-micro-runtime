/*
 * Copyright (c) 2023 Wind River Systems, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

int getentropy(void *buffer, size_t length)
{
    uint32_t count = 0;
    int fd = 0;

    if (!buffer || length > INT32_MAX) {
        errno = EINVAL;
        return -1;
    }

    if (length == 0) {
        return 0;
    }

    int flags = O_RDONLY;
#if defined(O_CLOEXEC)
    flags |= O_CLOEXEC;
#endif

    fd = open("/dev/urandom", flags, 0);
    if (fd < 0) {
        return -1;
    }

    while(count < length) {
        ssize_t ret = read(fd, (char*)buffer + count, length - count);
        if (ret <= 0) {
            if (errno!=EAGAIN && errno!=EINTR) break;
        }
        else {
          count += ret;
        }
    }
    close(fd);

    if (count != length) {
        errno = EFAULT;
        return -1;
    }

    return 0;
}

ssize_t pread(int fd, void * buf, size_t size, off_t offset) {
    off_t offs0;
    ssize_t rd;

    if ((offs0 = lseek(fd, 0, SEEK_CUR)) == (off_t)-1) {
        return -1;
    }
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        return -1;
    }

    rd = read(fd, (void *)buf, size);
    if (lseek(fd, offs0, SEEK_SET) == (off_t)-1) {
        return -1;
    }

    return rd;
}

ssize_t pwrite(int fd, const void * buf, size_t size, off_t offset) {
    off_t offs0;
    ssize_t wr;

    if ((offs0 = lseek(fd, 0, SEEK_CUR)) == (off_t)-1) {
        return -1;
    }
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
        return -1;
    }

    wr = write(fd, (void *)buf, size);
    if (lseek(fd, offs0, SEEK_SET) == (off_t)-1) {
        return -1;
    }

    return wr;
}

int openat(int fd, const char *path, int oflags, ...)
{
    errno = ENOSYS;
    return -1;
}

ssize_t readlinkat(int dirfd, const char *restrict pathname,
                        char *restrict buf, size_t bufsiz) {
    errno = ENOSYS;
    return -1;
}

int mkdirat(int dirfd, const char *pathname, mode_t mode) {
    errno = ENOSYS;
    return -1;
}

int linkat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags) {
    errno = ENOSYS;
    return -1;
}
int symlinkat(const char *target, int newdirfd, const char *linkpath) {
    errno = ENOSYS;
    return -1;
}

int fstatat(int fd, const char *restrict path,
           struct stat *restrict buf, int flag) {
    errno = ENOSYS;
    return -1;
}

DIR *fdopendir(int fd) {
    errno = ENOSYS;
    return NULL;
}
void seekdir(DIR *dirp, long loc) {
    errno = ENOSYS;
}

long telldir(DIR *dirp) {
    errno = ENOSYS;
    return -1;
}

int renameat(int olddirfd, const char *oldpath,
             int newdirfd, const char *newpath) {
    errno = ENOSYS;
    return -1;
}

int unlinkat(int dirfd, const char *pathname, int flags) {
    errno = ENOSYS;
    return -1;
}

int utimensat(int fd, const char *path, const struct timespec times[2], int flag) {
    errno = ENOSYS;
    return -1;
}

