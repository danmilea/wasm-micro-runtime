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
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <platform_internal.h>

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

ssize_t pread(int fd, void *buf, size_t size, off_t offset) {
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

ssize_t pwrite(int fd, const void *buf, size_t size, off_t offset) {
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

char* atCatPath(int fd, const char *path) {
    char *atPath;
    struct stat statbuf;

    /*
     * assume leading backslash indicates absolute filename
     */
    if (fd == AT_FDCWD || path[0] == '/') return NULL;

    if (fstat(fd, &statbuf) != 0) {
        errno = EBADF;
        return (char *)-1;
    }
    if (!S_ISDIR(statbuf.st_mode)) {
        errno = ENOTDIR;
        return (char *)-1;
    }

    atPath = malloc(PATH_MAX + 1);

    if (atPath) {
        ioctl(fd, FIOGETNAME, atPath);
        strcat_s(atPath, PATH_MAX, "/");
        strcat_s(atPath, PATH_MAX, path);
    }
    return atPath;
}

int openat(int dirfd, const char *path, int oflag, ...) {
    int ret;
    int mode;
    va_list vaList;
    char *newPath = atCatPath(dirfd, path);

    if (newPath == (char *)(-1)) return -1;

    va_start(vaList, oflag);
    mode = va_arg(vaList, int);
    va_end(vaList);

    if (newPath == NULL) return open(path, oflag, mode);

    ret = open(newPath, oflag, mode);
    free(newPath);
    return ret;
}

ssize_t readlinkat(int fd, const char *path, char *buf, size_t bufsize) {
    ssize_t ret;
    char *newPath = atCatPath(fd, path);

    if (newPath == (char *)(-1)) {
        return -1;
    }

    if (newPath == NULL) {
        return readlink(path, buf, bufsize);
    }

    ret = readlink(newPath, buf, bufsize);
    free(newPath);

    return ret;
}

int mkdirat(int fd, const char *path, mode_t mode) {
    char *newPath = atCatPath(fd, path);

    int res = mkdir(newPath, mode);
    free(newPath);

    return res;
}

int symlinkat(const char *path1, int fd, const char *path2) {
    int ret;

    char *newPath = atCatPath(fd, path2);

    if (newPath == (char *)(-1)) return -1;

    if (newPath == NULL) return symlink(path1, newPath);

    ret = symlink(path1, newPath);
    free(newPath);

    return ret;
}

int fstatat(int fd, const char *path, struct stat *buf, int flag) {
    int ret;
    char *newPath = atCatPath(fd, path);

    if (newPath == (char *)(-1)) {
        return -1;
    }

    if (flag == AT_SYMLINK_NOFOLLOW) {
        if (newPath == NULL) return lstat(path, buf);

        ret = lstat(newPath, buf);
    } else {
        if (newPath == NULL) return stat(path, buf);

        ret = stat(newPath, buf);
    }

    free(newPath);

    return ret;
}

int unlinkat(int fd, const char *path, int flag) {
    int res;
    char *newPath = atCatPath(fd, path);

    if (newPath == (char *)(-1)) {
        return -1;
    }

    if (flag == AT_REMOVEDIR) {
        if (newPath == NULL) res = rmdir(path);

        res = rmdir(newPath);
    } else {
        if (newPath == NULL) res = unlink(path);

        res = unlink(newPath);
    }

    free(newPath);

    return res;
}

int linkat(int oldfd, const char *oldpath, int newfd, const char *newpath, int flag) {
    int res;
    char *path1, *path2;
    char *realPath1 = NULL;

    char *_path1 = atCatPath(oldfd, oldpath);

    if (_path1 == (char *)(-1)) {
        return -1;
    }

    char *_path2 = atCatPath(newfd, newpath);

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

    if (flag == AT_SYMLINK_FOLLOW) {
        realPath1 = realpath(path1, NULL);
        res = link(realPath1, path2);
        free(realPath1);
    } else {
        res = link(path1, path2);
    }

    free(_path1);
    free(_path2);

    return res;
}

int utimensat(int fd, const char *path, const struct timespec times[2], int flag) {
    int res;
    struct timeval _times[2] = {
        { times[0].tv_sec, times[0].tv_nsec / 1000 },
        { times[0].tv_sec, times[0].tv_nsec / 1000 },
    };
    char *newPath = atCatPath(fd, path);

    if (newPath == (char *)(-1)) {
        return -1;
    }

    if (newPath == NULL) {
        return utimes(path, _times);
    }

    res = utimes(newPath, _times);

    free(newPath);

    return res;

}

DIR* fdopendir(int fd) {
    char *newPath = atCatPath(fd, "");

    DIR *ret = NULL;

    if (newPath == (char *)(-1)) {
        return NULL;
    }

    ret = opendir(newPath);
    free(newPath);

    return ret;
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

