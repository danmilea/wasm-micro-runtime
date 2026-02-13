/*
 * Copyright (c) 2023-2025 Wind River Systems, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform_internal.h>

static SEM_ID rwsem; // pread/pwrite mutex semaphore

int getentropy(void *buffer, size_t length) {
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

    while (count < length) {
        ssize_t ret = read(fd, (char *)buffer + count, length - count);
        if (ret <= 0) {
            if (errno != EAGAIN && errno != EINTR) break;
        } else {
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

ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
    long long curOffset;
    ssize_t   bytesRead;

    if ((!rwsem) && ((rwsem = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE)) == NULL)) {
        return -1;
    }

    if (semTake(rwsem, WAIT_FOREVER) == ERROR) {
        return -1;
    }

    if (ioctl(fd, FIOWHERE64, (_Vx_ioctl_arg_t)&curOffset) == ERROR) {
        (void)semGive(rwsem);
        return -1;
    }

    if (lseek(fd, offset, SEEK_SET) == ERROR) {
        (void)semGive(rwsem);
        return -1;
    }

    bytesRead = read(fd, buf, count);

    if (lseek(fd, curOffset, SEEK_SET) == ERROR) {
        (void)semGive(rwsem);
        return -1;
    }

    (void)semGive(rwsem);

    return bytesRead;
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) {
    long long curOffset;
    ssize_t   bytesWritten;

    if ((!rwsem) && ((rwsem = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE)) == NULL)) {
        return -1;
    }

    if (semTake(rwsem, WAIT_FOREVER) == ERROR) {
        return -1;
    }

    if (ioctl(fd, FIOWHERE64, (_Vx_ioctl_arg_t)&curOffset) == ERROR) {
        (void)semGive(rwsem);
        return -1;
    }

    if (lseek(fd, offset, SEEK_SET) == ERROR) {
        (void)semGive(rwsem);
        return -1;
    }

    bytesWritten = write(fd, buf, count);

    if (lseek(fd, curOffset, SEEK_SET) == ERROR) {
        (void)semGive(rwsem);
        return -1;
    }

    (void)semGive(rwsem);

    return bytesWritten;
}

static char* atCatPath
(
    int fd,
    const char *path
    ) {
    char *atPath;
    struct stat statbuf;
    size_t len;

    if (path == NULL) {
        errno = EINVAL;
        return (char *)-1;
    }

    /* 
     * assume leading backslash indicates absolute
     * filename
     */
    if (fd == AT_FDCWD || path[0] == '/') return NULL;

    if (fstat(fd, &statbuf) != 0) {
        return (char *)-1;
    }
    if (!S_ISDIR(statbuf.st_mode)) {
        errno = ENOTDIR;
        return (char *)-1;
    }

    atPath = malloc(PATH_MAX);

    if (atPath == NULL) {
        return (char *)-1;
    }

    if (ioctl(fd, FIOGETNAME, atPath) < 0) {
        free(atPath);
        return (char *)-1;
    }

    len = strnlen_s(atPath, PATH_MAX);

    if (len < PATH_MAX) {
        if (len != 0 & atPath[len - 1] != '/') {
            atPath[len++] = '/';
        }

        if (len < PATH_MAX) {
            if (strlcpy(atPath + len, path, PATH_MAX - len) < PATH_MAX - len) {
                return atPath;
            }
        }
    }

    free(atPath);
    errno = ENAMETOOLONG;
    return (char *)-1;
}

int renameat(int olddirfd, const char *oldpath,
             int newdirfd, const char *newpath) {
    int res;
    char *path1, *path2;

    char *_path1 = atCatPath(olddirfd, oldpath);

    if (_path1 == (char *)(-1)) {
        return -1;
    }

    char *_path2 = atCatPath(newdirfd, newpath);

    if (_path1 == (char *)(-1)) {
        return -1;
    }

    if (_path1 == NULL) {
        path1 = (char *)oldpath;
    } else {
        path1 = _path1;
    }

    if (_path2 == NULL) {
        path2 = (char *)newpath;
    } else {
        path2 = _path2;
    }

    res = rename(path1, path2);

    free(_path1);
    free(_path2);

    return res;
}

void seekdir(DIR *dirp, long loc) {
    printf("[vxworks/platform_internal.c] %s:%d - errno ENOSYS\n", __FUNCTION__, __LINE__);
    errno = ENOSYS;
}

long telldir(DIR *dirp) {
    printf("[vxworks/platform_internal.c] %s:%d - errno ENOSYS\n", __FUNCTION__, __LINE__);
    errno = ENOSYS;
    return -1;
}

